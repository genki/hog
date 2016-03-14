#include "hog.h"
#define MAX_CMD_NAME 16

// command table
static struct {
    char name[MAX_CMD_NAME];
    void (*handler)(server_t*, grn_ctx*);
} cmd_handlers[] = {
    {"ping", hog_ping},
    {"get", hog_get},
    {"put", hog_put},
    {"del", hog_del},
    {"exist", hog_exist},
    {"fin", hog_fin},
    {"count", hog_count},
};

void* server(void *arg)
{
    int loop = 1;
    char num_handlers = sizeof(cmd_handlers)/sizeof(cmd_handlers[0]);
    server_t *s = (server_t*)arg;
    fprintf(stdout, "connection opening: %d\n", s->socket);
    hog_t *hog = s->hog;
    grn_ctx ctx;
    grn_ctx_init(&ctx, 0);
    // open db
    grn_obj *db = grn_db_open(&ctx, hog->db_path);
    // setup socket
    struct timeval to;
    to.tv_sec = 10;
    to.tv_usec = 0;
    //setsockopt(s->socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&to, sizeof(to));
    setsockopt(s->socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&to, sizeof(to));
    // send the command list
    if(submit(s->socket, &num_handlers, 1) != 0){
        fprintf(stderr, "Failed to send num handlers\n");
    }
    for(char i = 0; i < num_handlers; ++i){
        const char *name = cmd_handlers[i].name;
        if(submit_chunk(s->socket, name) != 0){
            fprintf(stderr, "Failed to send cmd (%d) %s\n", i, name);
        }
    }
    while(loop){
        unsigned char cmd;
        if(receive(s->socket, &cmd, 1) != 0){
            switch(errno){
            case 0: case EBADF: case ENOENT: case EAGAIN: break;
            case ETIMEDOUT: continue;
            default:
                fprintf(stderr, "Failed to recv cmd: %s\n", strerror(errno));
                break;
            }
            loop = 0;
            break;
        }
        if(cmd < num_handlers) (cmd_handlers[cmd].handler)(s, &ctx);
        else fprintf(stderr, "Invalid cmd: %d\n", cmd);
    }
cleanup:
    fprintf(stdout, "connection closing: %d\n", s->socket);
    GRN_OBJ_FIN(&ctx, db);
    grn_ctx_fin(&ctx);
    close(s->socket);
    free(s);
    return NULL;
}
