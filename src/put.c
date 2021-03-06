#include "hog.h"

// <cmd> {<len> <column id>} <types> <#kvs> [{<len> <key>} {<len> <value>}]...
void put_or_set(server_t *s, grn_ctx *ctx, int set)
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
    // receive keys and values
    uint32_t nkeys;
    HOG_RECV(s, &nkeys, sizeof(nkeys), goto cleanup);
    nkeys = ntohl(nkeys);
    grn_obj value;
    GRN_OBJ_INIT(&value, GRN_BULK, 0, types[1]);
    for(uint32_t i = 0; i < nkeys; ++i){
        HOG_RECV(s, &len, sizeof(len), goto value_fin);
        len = ntohl(len);
        buf = hog_alloc(buf, len);
        HOG_RECV(s, buf, len, goto value_fin);
        ntoh_buf(buf, len, types[0]);
        grn_id id;
        if(set){
            id = grn_table_get(ctx, table, buf, len);
        }else{
            id = grn_table_add(ctx, table, buf, len, NULL);
        }
        HOG_RECV(s, &len, sizeof(len), goto value_fin);
        len = ntohl(len);
        buf = hog_alloc(buf, len);
        HOG_RECV(s, buf, len, goto value_fin);
        if(id == GRN_ID_NIL) continue;
        ntoh_buf(buf, len, types[1]);
        GRN_BULK_REWIND(&value);
        grn_bulk_write(ctx, &value, buf, len);
        grn_obj_set_value(ctx, col, id, &value, GRN_OBJ_SET);
    }
    submit_one(s->socket);
value_fin:
    GRN_OBJ_FIN(ctx, &value);
cleanup:
    free(buf);
}

// add new record if there is no record for the keys.
void hog_put(server_t *s, grn_ctx *ctx)
{
    put_or_set(s, ctx, 0);
}

// do nothing if there is no record yet for the keys.
void hog_set(server_t *s, grn_ctx *ctx)
{
    put_or_set(s, ctx, 1);
}
