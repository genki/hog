#include "hog.h"

// Put multiple values for a key to a table at once.
// <cmd> {<len> <table name>} <key type> <len> <key>
// <#cols> [<value type> <len> <column id> <len> <value>]...
void hog_store(server_t *s, grn_ctx *ctx)
{
    uint32_t len;
    receive(s->socket, &len, sizeof(len));
    len = ntohl(len);
    char *buf = malloc(len);
    receive(s->socket, buf, len);
    grn_obj *table = grn_ctx_get(ctx, buf, len);
    // get key type, len and key
    char type;
    receive(s->socket, &type, 1);
    receive(s->socket, &len, sizeof(len));
    len = ntohl(len);
    buf = realloc(buf, len);
    receive(s->socket, buf, len);
    ntoh_buf(buf, len, type);
    grn_id id = grn_table_add(ctx, table, buf, len, NULL);
    // get columns and values
    uint32_t ncols;
    receive(s->socket, &ncols, sizeof(ncols));
    ncols = ntohl(ncols);
    grn_obj value;
    GRN_VOID_INIT(&value);
    for(uint32_t i = 0; i < ncols; ++i){
        receive(s->socket, &type, 1);
        receive(s->socket, &len, sizeof(len));
        len = ntohl(len);
        buf = realloc(buf, len);
        receive(s->socket, buf, len);
        grn_obj *col = grn_obj_column(ctx, table, buf, len);
        receive(s->socket, &len, sizeof(len));
        len = ntohl(len);
        buf = realloc(buf, len);
        receive(s->socket, buf, len);
        ntoh_buf(buf, len, type);
        grn_obj_reinit(ctx, &value, type, 0);
        grn_bulk_write(ctx, &value, buf, len);
        grn_obj_set_value(ctx, col, id, &value, GRN_OBJ_SET);
    }
    GRN_OBJ_FIN(ctx, &value);
cleanup:
    free(buf);
}
