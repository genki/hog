#include "hog.h"

// <cmd> {<len> <column id>} <type> <#keys> [{<len> <key>}]...
void hog_del(server_t *s, grn_ctx *ctx)
{
    uint32_t len;
    HOG_RECV(s, &len, sizeof(len), return);
    len = ntohl(len);
    char *buf = hog_alloc(NULL, len);
    HOG_RECV(s, buf, len, goto cleanup);
    grn_obj *table = grn_ctx_get(ctx, buf, len);
    if(!grn_obj_is_table(ctx, table)) table = NULL;
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
        buf = hog_alloc(buf, len);
        HOG_RECV(s, buf, len, goto cleanup);
        if(table){
            ntoh_buf(buf, len, type);
            grn_table_delete(ctx, table, buf, len);
        }
    }
    submit_one(s->socket);
cleanup:
    free(buf);
}
