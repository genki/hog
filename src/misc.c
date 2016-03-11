#include "hog.h"

// <cmd>
void hog_ping(server_t *s, grn_ctx *ctx)
{
    char one = 1;
    submit(s->socket, &one, 1);
}

// <cmd>
void hog_fin(server_t *s, grn_ctx *ctx)
{
    close(s->socket);
}
