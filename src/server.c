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
    {"fetch", hog_fetch},
    {"store", hog_store},
    {"find", hog_find},
};

void* server(void *arg)
{
    char num_handlers = sizeof(cmd_handlers)/sizeof(cmd_handlers[0]);
    server_t *s = (server_t*)arg;
    s->running = 1;
    fprintf(stdout, "connection opening: %d\n", s->socket);
    hog_t *hog = s->hog;
    grn_rc rc;
    grn_ctx ctx;
    rc = grn_ctx_init(&ctx, 0);
    if(rc != GRN_SUCCESS){
        fprintf(stderr, "Failed to init ctx: %d\n", rc);
        goto cleanup;
    }
    // open db
    grn_obj *db = grn_db_open(&ctx, hog->db_path);
    if(db == NULL){
        fprintf(stderr, "Failed to open db: %s\n", hog->db_path);
        goto ctx_fin;
    }
    // setup socket
    struct timeval to;
    to.tv_sec = 10;
    to.tv_usec = 0;
    setsockopt(s->socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&to, sizeof(to));
    setsockopt(s->socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&to, sizeof(to));
    // send the command list
    if(submit(s->socket, &num_handlers, 1) != 0){
        fprintf(stderr, "Failed to send num handlers\n");
        goto ctx_fin;
    }
    for(char i = 0; i < num_handlers; ++i){
        const char *name = cmd_handlers[i].name;
        if(submit_chunk(s->socket, name) != 0){
            fprintf(stderr, "Failed to send cmd (%d) %s\n", i, name);
            goto ctx_fin;
        }
    }
    int retry_count = 0;
    while(s->running){
        unsigned char cmd;
        if(receive(s->socket, &cmd, 1) != 0){
            switch(errno){
            case 0: case EBADF: case ENOENT: case EAGAIN: break;
            case ETIMEDOUT:
                if(retry_count++ > 3) break;
                else continue;
            default:
                fprintf(stderr, "Failed to recv cmd: %s\n", strerror(errno));
                break;
            }
            s->running = 0;
            break;
        }
        retry_count = 0;
        if(cmd < num_handlers){
            if(hog->verbose){
                fprintf(stdout, "exec %s: %d\n",
                        cmd_handlers[cmd].name, s->socket);
            }
            (cmd_handlers[cmd].handler)(s, &ctx);
        }else{
            fprintf(stderr, "Invalid cmd: %d\n", cmd);
            break;
        }
    }
db_fin:
    fprintf(stdout, "connection closing: %d\n", s->socket);
    GRN_OBJ_FIN(&ctx, db);
ctx_fin:
    grn_ctx_fin(&ctx);
cleanup:
    close(s->socket);
    free(s);
    return NULL;
}
