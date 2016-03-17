#include "hog.h"

// Get multiple values for a key from a table at once.
// <cmd> {<len> <table name>} <key type> <len> <key>
// <#cols> [<value type> <len> <column id>]...
void hog_fetch(server_t *s, grn_ctx *ctx)
{
    uint32_t len;
    HOG_RECV(s, &len, sizeof(len), return);
    len = ntohl(len);
    char *buf = malloc(len);
    HOG_RECV(s, buf, len, goto cleanup);
    grn_obj *table = grn_ctx_get(ctx, buf, len);
    // get key type, len and key
    char type;
    HOG_RECV(s, &type, 1, goto cleanup);
    HOG_RECV(s, &len, sizeof(len), goto cleanup);
    len = ntohl(len);
    buf = realloc(buf, len);
    HOG_RECV(s, buf, len, goto cleanup);
    ntoh_buf(buf, len, type);
    grn_id id = grn_table_get(ctx, table, buf, len);
    // get columns and submit values for each column
    uint32_t ncols;
    HOG_RECV(s, &ncols, sizeof(ncols), goto cleanup);
    ncols = ntohl(ncols);
    grn_obj value;
    GRN_VOID_INIT(&value);
    for(uint32_t i = 0; i < ncols; ++i){
        HOG_RECV(s, &type, 1, goto value_fin);
        HOG_RECV(s, &len, sizeof(len), goto value_fin);
        len = ntohl(len);
        buf = realloc(buf, len);
        HOG_RECV(s, buf, len, goto value_fin);
        if(id != GRN_ID_NIL){
            grn_obj *col = grn_obj_column(ctx, table, buf, len);
            GRN_BULK_REWIND(&value);
            grn_obj_get_value(ctx, col, id, &value);
            if(ctx->rc == GRN_SUCCESS){
                void *bulk = GRN_BULK_HEAD(&value);
                uint32_t blen = GRN_BULK_VSIZE(&value);
                uint32_t nblen = htonl(blen);
                HOG_SEND(s, &nblen, sizeof(nblen), goto value_fin);
                hton_buf(bulk, blen, type);
                HOG_SEND(s, bulk, blen, goto value_fin);
                continue;
            }
        }
        uint32_t zero = htonl(0);
        HOG_SEND(s, &zero, sizeof(zero), goto value_fin);
    }
value_fin:
    GRN_OBJ_FIN(ctx, &value);
cleanup:
    free(buf);
}
