#include "hog.h"

// Find keys that records have specified values 
// <cmd> {<len> <column id>} <key type> <value type> <offset> <limit>
// if <value type> is 0, only keys are retrieved.
void hog_each(server_t *s, grn_ctx *ctx)
{
    uint32_t len;
    HOG_RECV(s, &len, sizeof(len), return);
    len = ntohl(len);
    char *buf = hog_alloc(NULL, len);
    HOG_RECV(s, buf, len, goto cleanup);
    grn_obj *table, *col;
    col = grn_ctx_get(ctx, buf, len);
    if(grn_obj_is_table(ctx, col)) table = col;
    else table = grn_column_table(ctx, col);
    // get key and value types
    char types[2];
    HOG_RECV(s, types, 2, goto cleanup);
    // get offset and limit
    int32_t offset, limit;
    HOG_RECV(s, &offset, sizeof(offset), goto cleanup);
    offset = ntohl(offset);
    HOG_RECV(s, &limit, sizeof(limit), goto cleanup);
    limit = ntohl(limit);
    // open cursor
    grn_table_cursor *cursor = grn_table_cursor_open(ctx, table,
            NULL, 0, NULL, 0, offset, limit, GRN_CURSOR_BY_ID); 
    grn_obj value;
    if (types[1]) GRN_VOID_INIT(&value);
    if(cursor) for(;;) {
        grn_id id = grn_table_cursor_next(ctx, cursor);
        if(id == GRN_ID_NIL) break;
        void *key;
        uint32_t blen = grn_table_cursor_get_key(ctx, cursor, (void**)&key);
        uint32_t nblen = htonl(blen);
        HOG_SEND(s, &nblen, sizeof(nblen), break);
        hton_buf(key, blen, types[0]);
        HOG_SEND(s, key, blen, break);
        if(types[1]){
            GRN_BULK_REWIND(&value);
            grn_obj_get_value(ctx, col, id, &value);
            if(ctx->rc == GRN_SUCCESS){
                void *bulk = GRN_BULK_HEAD(&value);
                blen = GRN_BULK_VSIZE(&value);
                nblen = htonl(blen);
                HOG_SEND(s, &nblen, sizeof(nblen), break);
                hton_buf(bulk, blen, types[1]);
                HOG_SEND(s, bulk, blen, break);
                continue;
            }
            uint32_t zero = htonl(0);
            HOG_SEND(s, &zero, sizeof(zero), break);
        }
    }
    uint32_t last = htonl(0xFFFFFFFF);
    HOG_SEND(s, &last, sizeof(last), goto value_fin);
value_fin:
    if (types[1]) GRN_OBJ_FIN(ctx, &value);
cursor_fin:
    if(cursor) grn_table_cursor_close(ctx, cursor);
cleanup:
    free(buf);
}
