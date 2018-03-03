#include "hog.h"
#define MAX_CMD_NAME 16

__thread server_t *server_self;

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
    {"set", hog_set},
    {"query", hog_query},
    {"each", hog_each},
};

void* server(void *arg)
{
    char num_handlers = sizeof(cmd_handlers)/sizeof(cmd_handlers[0]);
    server_t *s = server_self = (server_t*)arg;
    s->running = 1;
    s->error = 0;
    hog_t *hog = s->hog;
    if(hog->verbose){
        fprintf(stdout, "connection opening: %d\n", s->socket);
    }
    grn_rc rc;
    grn_ctx ctx;
    rc = grn_ctx_init(&ctx, 0);
    if(rc != GRN_SUCCESS){
        fprintf(stderr, "Failed to init ctx: (%d) %s\n",
            rc, grn_rc_to_string(rc));
        goto cleanup;
    }
    rc = grn_ctx_use(&ctx, hog->db);
    if(rc != GRN_SUCCESS){
        fprintf(stderr, "Failed to use db: (%d) %s\n",
            rc, grn_rc_to_string(rc));
        goto ctx_fin;
    }

    // setup socket
    struct timeval to;
    to.tv_sec = 10;
    to.tv_usec = 0;
    setsockopt(s->socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&to, sizeof(to));
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
            if(s->error){
                fprintf(stderr, "error: (%d: %s) in exec %s: %d\n",
                    s->error, strerror(s->error),
                    cmd_handlers[cmd].name, s->socket);
                break;
            }
        }else{
            fprintf(stderr, "Invalid cmd: %d\n", cmd);
            break;
        }
    }
ctx_fin:
    grn_ctx_fin(&ctx);
cleanup:
    if(s->killed){
        fprintf(stdout, "connection closed: %d\n", s->socket);
    }else{
        if(hog->verbose){
            fprintf(stdout, "connection closing: %d\n", s->socket);
        }
        close(s->socket);
        if(s->running){
            fprintf(stderr, "connection aborted: %d\n", s->socket);
        }
    }
    pthread_mutex_lock(&hog->mutex);
    pthread_t *self = &hog->threads[s->thread_id];
    pthread_t *tail = &hog->threads[--hog->nservers];
    *self = *tail;
    server_t *s_tail = hog->servers[hog->nservers];
    s_tail->thread_id = s->thread_id;
    hog->servers[s->thread_id] = s_tail;
    pthread_mutex_unlock(&hog->mutex);
    free(s);
    return NULL;
}
