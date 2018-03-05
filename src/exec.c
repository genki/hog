#include "hog.h"

static void
on_exec_output(grn_ctx *ctx, int flags, void *arg)
{
    server_t *s = (server_t*)arg;
    char *buf = NULL;
    unsigned int len = 0;
    int recv_flags;
    grn_ctx_recv(ctx, &buf, &len, &recv_flags);
    uint32_t nblen = htonl(len);
    HOG_SEND(s, &nblen, sizeof(nblen), return);
    if(len > 0){
        HOG_SEND(s, buf, len, return);
    }
}

// Execute groonga command.
// <cmd> {<len> <command>}
void hog_exec(server_t *s, grn_ctx *ctx)
{
    uint32_t len;
    HOG_RECV(s, &len, sizeof(len), return);
    len = ntohl(len);
    char *buf = hog_alloc(NULL, len);
    HOG_RECV(s, buf, len, goto cleanup);
    grn_ctx_recv_handler_set(ctx, on_exec_output, s);
    grn_ctx_send(ctx, buf, len, 0);
    if(ctx->stat == GRN_CTX_QUIT) s->running = 0;
cleanup:
    free(buf);
}
