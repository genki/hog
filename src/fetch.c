#include "hog.h"

// Get multiple values for a key from a table at once.
// <cmd> {<len> <table name>} <key type> <len> <key>
// <#cols> [<value type> <len> <column id>]...
void hog_fetch(server_t *s, grn_ctx *ctx)
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
    grn_id id = grn_table_get(ctx, table, buf, len);
    // get columns and submit values for each column
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
        if(id != GRN_ID_NIL){
            grn_obj *col = grn_obj_column(ctx, table, buf, len);
            GRN_BULK_REWIND(&value);
            grn_obj_get_value(ctx, col, id, &value);
            if(ctx->rc == GRN_SUCCESS){
                void *bulk = GRN_BULK_HEAD(&value);
                uint32_t blen = GRN_BULK_VSIZE(&value);
                uint32_t nblen = htonl(blen);
                submit(s->socket, &nblen, sizeof(nblen));
                hton_buf(bulk, blen, type);
                submit(s->socket, bulk, blen);
                continue;
            }
        }
        uint32_t zero = htonl(0);
        submit(s->socket, &zero, sizeof(zero));
    }
    GRN_OBJ_FIN(ctx, &value);
cleanup:
    free(buf);
}
