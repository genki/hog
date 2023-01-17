#include "hog.h"

hog_t hog = {0};
extern __thread server_t *server_self;

void on_signal(int signo){
    switch(signo){
    case SIGINT: fprintf(stdout, "INT signal received.\n"); break;
    case SIGTERM: fprintf(stdout, "TERM signal received.\n"); break;
    default: return;
    }
    close(hog.socket);
}

void on_close(int USR1)
{
    server_self->killed = 1;
    close(server_self->socket);
}

void cleanup()
{
    if(hog.ctx){
        if(hog.db) GRN_OBJ_FIN(hog.ctx, hog.db);
        grn_ctx_fin(hog.ctx);
        free(hog.ctx);
    }
    grn_fin();
    pthread_mutex_destroy(&hog.mutex);
    free(hog.servers);
    free(hog.threads);
    fprintf(stdout, "hog server successfully stopped.\n");
}

int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IONBF, 0);
    const char *db_path = NULL;
    const char *address = "0.0.0.0"; 
    const char *log_path = "/dev/null";
    const char *log_level = NULL;
    int port = 18618;
    int max_conn = 1024;
    int lock_clear = 0;
    int lock_timeout = 10000; // in [ms]
    hog.verbose = 0;

    // opt parse
    for(int i = 1; i < argc; ++i){
        const char *arg = argv[i];
        if(arg[0] == '-'){
            switch(arg[1]){
            case 'b': address = argv[++i]; break;
            case 'p': port = atoi(argv[++i]); break;
            case 'c': max_conn = atoi(argv[++i]); break;
            case 't': lock_timeout = atoi(argv[++i]); break;
            case 'V': hog.verbose = 1; break;
            case 'u': lock_clear = 1; break;
            case 'l': log_path = argv[++i]; break;
            case 'L': log_level = argv[++i]; break;
            case 'v':
                fprintf(stdout, "hog-%s with Groonga-%s\n",
                    HOG_VERSION, GRN_VERSION);
                exit(EXIT_SUCCESS);
            default:
                fprintf(stderr, "Unknown option -%s\n", &arg[1]);
                exit(EXIT_FAILURE);
            }
        }else db_path = arg;
    }

    // check args
    if(db_path == NULL){
        fprintf(stderr, "No DB path\n");
        exit(EXIT_FAILURE);
    }

    // start server
    struct sockaddr_in saddr = {0}, caddr = {0};
    unsigned int len = sizeof(struct sockaddr_in);
    hog.socket = socket(AF_INET, SOCK_STREAM, 0);
    saddr.sin_family = PF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(port);
    int opt = 1;
    setsockopt(hog.socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(hog.socket, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
    bind(hog.socket, (struct sockaddr*)&saddr, len);
    listen(hog.socket, max_conn);
    fprintf(stdout, "hog server started listening port #%d...\n", port);

    // init server table
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    hog.servers = calloc(max_conn, sizeof(server_t));
    hog.threads = calloc(max_conn, sizeof(pthread_t));
    hog.nservers = 0;
    pthread_mutex_init(&hog.mutex, 0);

    // install signal handlers
    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);
    signal(SIGUSR1, on_close);

    // setup logger
    grn_default_logger_set_path(log_path);
    if (log_level) {
      grn_log_level level = GRN_LOG_DEFAULT_LEVEL;
      if (!grn_log_level_parse(log_level, &level)){
        fprintf(stderr, "Failed to parse log level: <%s>\n", log_level);
        exit(EXIT_FAILURE);
      }
      grn_default_logger_set_max_level(level);
    }

    // init groonga
    grn_init();
    grn_set_lock_timeout(lock_timeout);
    atexit(cleanup);
    hog.ctx = hog_alloc(NULL, sizeof(grn_ctx));
    grn_rc rc = grn_ctx_init(hog.ctx, 0);
    if(rc != GRN_SUCCESS){
        fprintf(stderr, "Failed to init ctx: (%d) %s\n",
            rc, grn_rc_to_string(rc));
        free(hog.ctx);
        hog.ctx = NULL;
        goto cleanup;
    }
    hog.db = grn_db_open(hog.ctx, db_path);
    if(hog.db == NULL){
        fprintf(stderr, "Failed to open db: %s\n", db_path);
        goto cleanup;
    }

    // lock_clear
    if(lock_clear){
        grn_obj_clear_lock(hog.ctx, hog.db);
    }

    // accept loop
    fprintf(stdout, "hog server started accepting connections...\n");
    while(1){
        int c = accept(hog.socket, (struct sockaddr*)&caddr, &len);
        if(c < 0) break;
        pthread_mutex_lock(&hog.mutex);
        int tid = hog.nservers++;
        server_t *s = hog.servers[tid] = hog_alloc(NULL, sizeof(server_t));
        s->socket = c;
        s->hog = &hog;
        s->thread_id = tid;
        s->killed = 0;
        int ret = pthread_create(&hog.threads[tid], NULL, server, s);
        if(ret != 0){
            fprintf(stderr, "Failed to spawn thread %d (#%d)\n",
                s->thread_id, hog.nservers);
            close(c);
            free(s);
            hog.nservers--;
        }
        pthread_mutex_unlock(&hog.mutex);
    }

cleanup:
    fprintf(stdout, "waiting for %d servers...\n", hog.nservers);
    pthread_mutex_lock(&hog.mutex);
    for(int i = 0; i < hog.nservers; ++i){
        pthread_kill(hog.threads[i], SIGUSR1);
    }
    pthread_mutex_unlock(&hog.mutex);
    pthread_attr_destroy(&attr);
    pthread_exit(NULL);
    return EXIT_SUCCESS;
}
