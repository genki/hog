#include "hog.h"

// <cmd> {<len> <column id>} <type> <#keys> [{<len> <key>}]...
void hog_exist(server_t *s, grn_ctx *ctx)
{
    uint32_t len;
    HOG_RECV(s, &len, sizeof(len), return);
    len = ntohl(len);
    char *buf = malloc(len);
    HOG_RECV(s, buf, len, goto cleanup);
    grn_obj *col, *table;
    col = grn_ctx_get(ctx, buf, len);
    if(grn_obj_is_table(ctx, col)) table = col;
    else table = grn_column_table(ctx, col);
    // get key type
    char type;
    HOG_RECV(s, &type, 1, goto cleanup);
    // submit values for each keys
    uint32_t nkeys;
    HOG_RECV(s, &nkeys, sizeof(nkeys), goto cleanup);
    nkeys = ntohl(nkeys);
    for(uint32_t i = 0; i < nkeys; ++i){
        HOG_RECV(s, &len, sizeof(len), goto cleanup);
        len = ntohl(len);
        buf = hog_realloc(buf, len);
        HOG_RECV(s, buf, len, goto cleanup);
        ntoh_buf(buf, len, type);
        grn_id id = grn_table_get(ctx, table, buf, len);
        char flag = (id == GRN_ID_NIL ? 0 : 1);
        submit(s->socket, &flag, 1);
    }
cleanup:
    free(buf);
}
