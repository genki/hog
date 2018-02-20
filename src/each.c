#include "hog.h"

// Find keys that records have specified values 
// <cmd> {<len> <column id>} <key type> <offset> <limit>
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
    // get key type
    char type;
    HOG_RECV(s, &type, 1, goto cleanup);
    // get offset and limit
    int32_t offset, limit;
    HOG_RECV(s, &offset, sizeof(offset), goto cleanup);
    offset = ntohl(offset);
    HOG_RECV(s, &limit, sizeof(limit), goto cleanup);
    limit = ntohl(limit);
    // open cursor
    grn_table_cursor *cursor = grn_table_cursor_open(ctx, table,
            NULL, 0, NULL, 0, offset, limit, GRN_CURSOR_BY_ID); 
    if(cursor) while(grn_table_cursor_next(ctx, cursor) != GRN_ID_NIL){
        void *key;
        uint32_t blen = grn_table_cursor_get_key(ctx, cursor, (void**)&key);
        uint32_t nblen = htonl(blen);
        HOG_SEND(s, &nblen, sizeof(nblen), break);
        hton_buf(key, blen, type);
        HOG_SEND(s, key, blen, break);
    }
    uint32_t last = htonl(0xFFFFFFFF);
    HOG_SEND(s, &last, sizeof(last), goto cursor_fin);
cursor_fin:
    if(cursor) grn_table_cursor_close(ctx, cursor);
cleanup:
    free(buf);
}
