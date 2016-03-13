#include "hog.h"

hog_t hog;

void on_signal(int signo){
    switch(signo){
    case SIGINT:
        fprintf(stdout, "INT signal received.\n");
        kill(0, SIGTERM);
        break;
    case SIGTERM:
        fprintf(stdout, "TERM signal received.\n");
        close(hog.socket);
        break;
    }
}

void cleanup()
{
    grn_fin();
}

int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IONBF, 0);
    hog.db_path = NULL;
    hog.bind = "0.0.0.0"; 
    hog.port = 18618;
    hog.max_conn = 1024;
    hog.num_threads = 4;

    // opt parse
    for(int i = 1; i < argc; ++i){
        const char *arg = argv[i];
        if(arg[0] == '-'){
            switch(arg[1]){
            case 't': hog.num_threads = atoi(argv[++i]); break;
            case 'b': hog.bind = argv[++i]; break;
            case 'p': hog.port = atoi(argv[++i]); break;
            case 'c': hog.max_conn = atoi(argv[++i]); break;
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

    // init groonga
    grn_init();
    atexit(cleanup);
    grn_set_lock_timeout(3*60*1000); // 3 min
    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);

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
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    while(1){
        int c = accept(hog.socket, (struct sockaddr*)&caddr, &len);
        if(c < 0) break;
        pthread_t thread;
        server_t *s = malloc(sizeof(s));
        s->socket = c;
        s->hog = &hog;
        if(pthread_create(&thread, &attr, server, s) != 0){
            fprintf(stderr, "Failed to spawn thread\n");
            free(s);
            close(c);
        }
    }
    pthread_attr_destroy(&attr);
    pthread_exit(NULL);
    fprintf(stdout, "hog server successfully stopped.\n");
    return EXIT_SUCCESS;
}
