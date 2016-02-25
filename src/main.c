#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include "hog.h"

static hog_t hog;

void on_signal(int signo){
    switch(signo){
    case SIGINT:
        printf("TERM signal received\n");
        close(hog.socket);
        break;
    }
}

int main(int argc, char *argv[])
{
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
    grn_ctx_init(&hog.ctx, 0);
    hog.db = grn_db_open(&hog.ctx, hog.db_path);
    signal(SIGINT, on_signal);

    // start server
    struct sockaddr_in saddr = {0}, caddr = {0};
    unsigned int len = sizeof(struct sockaddr_in);
    hog.socket = socket(AF_INET, SOCK_STREAM, 0);
    saddr.sin_family = PF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(hog.port);
    bind(hog.socket, (struct sockaddr*)&saddr, len);
    listen(hog.socket, hog.max_conn);
    printf("hog server started listening port #%d...\n", hog.port);
    while(1){
        int c = accept(hog.socket, (struct sockaddr*)&caddr, &len);
        if(c < 0) break;
    }

    // clean up
    grn_obj_close(&hog.ctx, hog.db);
    grn_ctx_fin(&hog.ctx);
    grn_fin();
    printf("hog server successfully stopped.\n");
    return EXIT_SUCCESS;
}
