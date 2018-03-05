#include "hog.h"

// Find keys that records have specified values 
// <cmd> {<len> <column id>} <key type>
// <#cols> [<value type> <len> <column id>]...
// <offset> <limit>
// if <#cols> is 0, only keys are retrieved.
void hog_each(server_t *s, grn_ctx *ctx)
{
    char *types = NULL;
    grn_obj value, **cols = NULL;
    uint32_t len;
    HOG_RECV(s, &len, sizeof(len), return);
    len = ntohl(len);
    char *buf = hog_alloc(NULL, len);
    HOG_RECV(s, buf, len, goto cleanup);
    grn_obj *table = grn_ctx_get(ctx, buf, len);
    // get key and value types
    char type;
    HOG_RECV(s, &type, 1, goto cleanup);
    // get columns and submit values for each column
    uint32_t ncols;
    HOG_RECV(s, &ncols, sizeof(ncols), goto cleanup);
    ncols = ntohl(ncols);
    if(ncols){
        GRN_VOID_INIT(&value);
        types = (char*)hog_alloc(NULL, ncols);
        cols = (grn_obj**)hog_alloc(NULL, ncols*sizeof(grn_obj*));
    }
    for(uint32_t i = 0; i < ncols; ++i){
        HOG_RECV(s, &types[i], 1, goto cols_fin);
        HOG_RECV(s, &len, sizeof(len), goto cols_fin);
        len = ntohl(len);
        buf = hog_alloc(buf, len);
        HOG_RECV(s, buf, len, goto cols_fin);
        cols[i] = grn_obj_column(ctx, table, buf, len);
    }
    // get offset and limit
    int32_t offset, limit;
    HOG_RECV(s, &offset, sizeof(offset), goto cols_fin);
    offset = ntohl(offset);
    HOG_RECV(s, &limit, sizeof(limit), goto cols_fin);
    limit = ntohl(limit);
    // open cursor
    grn_table_cursor *cursor = grn_table_cursor_open(ctx, table,
            NULL, 0, NULL, 0, offset, limit, GRN_CURSOR_BY_ID); 
    if(cursor) for(;;) {
        grn_id id = grn_table_cursor_next(ctx, cursor);
        if(id == GRN_ID_NIL) break;
        void *key;
        uint32_t blen = grn_table_cursor_get_key(ctx, cursor, (void**)&key);
        uint32_t nblen = htonl(blen);
        HOG_SEND(s, &nblen, sizeof(nblen), break);
        hton_buf(key, blen, type);
        HOG_SEND(s, key, blen, break);
        for(uint32_t i = 0; i < ncols; ++i){
            GRN_BULK_REWIND(&value);
            grn_obj_get_value(ctx, cols[i], id, &value);
            if(ctx->rc == GRN_SUCCESS){
                void *bulk = GRN_BULK_HEAD(&value);
                blen = GRN_BULK_VSIZE(&value);
                nblen = htonl(blen);
                HOG_SEND(s, &nblen, sizeof(nblen), break);
                hton_buf(bulk, blen, types[i]);
                HOG_SEND(s, bulk, blen, break);
                continue;
            }
            uint32_t zero = htonl(0);
            HOG_SEND(s, &zero, sizeof(zero), break);
        }
    }
    uint32_t last = htonl(0xFFFFFFFF);
    HOG_SEND(s, &last, sizeof(last), goto cursor_fin);
cursor_fin:
    if(cursor) grn_table_cursor_close(ctx, cursor);
cols_fin:
    if(ncols){
        free(cols);
        free(types);
        GRN_OBJ_FIN(ctx, &value);
    }
cleanup:
    free(buf);
}
