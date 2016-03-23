#include "hog.h"

// <cmd> {<len> <column id>} <types> <#keys> [{<len> <key>}]...
void hog_get(server_t *s, grn_ctx *ctx)
{
    uint32_t len;
    HOG_RECV(s, &len, sizeof(len), return);
    len = ntohl(len);
    char *buf = hog_alloc(NULL, len);
    HOG_RECV(s, buf, len, goto cleanup);
    grn_obj *col, *table;
    col = grn_ctx_get(ctx, buf, len);
    if(grn_obj_is_table(ctx, col)) table = col;
    else table = grn_column_table(ctx, col);
    // get key and value types
    char types[2];
    HOG_RECV(s, types, 2, goto cleanup);
    // submit values for each keys
    uint32_t nkeys;
    HOG_RECV(s, &nkeys, sizeof(nkeys), goto cleanup);
    nkeys = ntohl(nkeys);
    grn_obj value;
    GRN_VOID_INIT(&value);
    for(uint32_t i = 0; i < nkeys; ++i){
        HOG_RECV(s, &len, sizeof(len), goto value_fin);
        len = ntohl(len);
        buf = hog_alloc(buf, len);
        HOG_RECV(s, buf, len, goto value_fin);
        ntoh_buf(buf, len, types[0]);
        grn_id id = grn_table_get(ctx, table, buf, len);
        if(id != GRN_ID_NIL){
            GRN_BULK_REWIND(&value);
            grn_obj_get_value(ctx, col, id, &value);
            if(ctx->rc == GRN_SUCCESS){
                void *bulk = GRN_BULK_HEAD(&value);
                uint32_t blen = GRN_BULK_VSIZE(&value);
                uint32_t nblen = htonl(blen);
                HOG_SEND(s, &nblen, sizeof(nblen), break);
                hton_buf(bulk, blen, types[1]);
                HOG_SEND(s, bulk, blen, break);
                continue;
            }
        }
        uint32_t zero = htonl(0);
        HOG_SEND(s, &zero, sizeof(zero), break);
    }
value_fin:
    GRN_OBJ_FIN(ctx, &value);
cleanup:
    free(buf);
}
