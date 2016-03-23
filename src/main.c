#include "hog.h"

hog_t hog;
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
    grn_fin();
    pthread_mutex_destroy(&hog.mutex);
    free(hog.servers);
    free(hog.threads);
    fprintf(stdout, "hog server successfully stopped.\n");
}

int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IONBF, 0);
    hog.db_path = NULL;
    hog.bind = "0.0.0.0"; 
    hog.port = 18618;
    hog.max_conn = 1024;
    hog.verbose = 0;

    // opt parse
    for(int i = 1; i < argc; ++i){
        const char *arg = argv[i];
        if(arg[0] == '-'){
            switch(arg[1]){
            case 'b': hog.bind = argv[++i]; break;
            case 'p': hog.port = atoi(argv[++i]); break;
            case 'c': hog.max_conn = atoi(argv[++i]); break;
            case 'V': hog.verbose = 1; break;
            case 'v':
              fprintf(stdout, "hog-%s\n", PROJECT_VERSION);
              exit(EXIT_SUCCESS);
            default:
                fprintf(stderr, "Unknown option -%s\n", &arg[1]);
                exit(EXIT_FAILURE);
            }
        }else hog.db_path = arg;
    }

    // check args
    if(hog.db_path == NULL){
        fprintf(stderr, "No DB path\n");
        exit(EXIT_FAILURE);
    }

    // start server
    struct sockaddr_in saddr = {0}, caddr = {0};
    unsigned int len = sizeof(struct sockaddr_in);
    hog.socket = socket(AF_INET, SOCK_STREAM, 0);
    saddr.sin_family = PF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(hog.port);
    int opt = 1;
    setsockopt(hog.socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(hog.socket, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
    bind(hog.socket, (struct sockaddr*)&saddr, len);
    listen(hog.socket, hog.max_conn);
    fprintf(stdout, "hog server started listening port #%d...\n", hog.port);

    // init server table
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    hog.servers = calloc(hog.max_conn, sizeof(server_t));
    hog.threads = calloc(hog.max_conn, sizeof(pthread_t));
    hog.nservers = 0;
    pthread_mutex_init(&hog.mutex, 0);

    // init groonga
    grn_init();
    atexit(cleanup);
    grn_set_lock_timeout(3*60*1000); // 3 min
    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);
    signal(SIGUSR1, on_close);

    // accept loop
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
        if(pthread_create(&hog.threads[tid], NULL, server, s) != 0){
            fprintf(stderr, "Failed to spawn thread %d\n", s->thread_id);
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
