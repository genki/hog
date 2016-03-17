#include "hog.h"

// Put multiple values for a key to a table at once.
// <cmd> {<len> <table name>} <key type> <len> <key>
// <#cols> [<value type> <len> <column id> <len> <value>]...
void hog_store(server_t *s, grn_ctx *ctx)
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
    grn_id id = grn_table_add(ctx, table, buf, len, NULL);
    // get columns and values
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
        grn_obj *col = grn_obj_column(ctx, table, buf, len);
        HOG_RECV(s, &len, sizeof(len), goto value_fin);
        len = ntohl(len);
        buf = realloc(buf, len);
        HOG_RECV(s, buf, len, goto value_fin);
        ntoh_buf(buf, len, type);
        grn_obj_reinit(ctx, &value, type, 0);
        grn_bulk_write(ctx, &value, buf, len);
        grn_obj_set_value(ctx, col, id, &value, GRN_OBJ_SET);
    }
value_fin:
    GRN_OBJ_FIN(ctx, &value);
cleanup:
    free(buf);
}
