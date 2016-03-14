#include "hog.h"

// <cmd>
void hog_ping(server_t *s, grn_ctx *ctx)
{
    char one = 1;
    submit(s->socket, &one, 1);
}

void hog_count(server_t *s, grn_ctx *ctx)
{
    uint32_t len;
    receive(s->socket, &len, sizeof(len));
    len = ntohl(len);
    char *buf = malloc(len);
    receive(s->socket, buf, len);
    grn_obj *col, *table;
    col = grn_ctx_get(ctx, buf, len);
    if(grn_obj_is_table(ctx, col)) table = col;
    else table = grn_column_table(ctx, col);
    uint32_t size = htonl(grn_table_size(ctx, table));
    submit(s->socket, &size, sizeof(size));
cleanup:
    free(buf);
}

// <cmd>
void hog_fin(server_t *s, grn_ctx *ctx)
{
    s->running = 0;
    hog_ping(s, ctx);
}
