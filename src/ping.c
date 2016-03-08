#include "hog.h"

// <cmd>
void hog_ping(server_t *s, grn_ctx *ctx)
{
    char one = 1;
    submit(s->socket, &one, 1);
}
