#include "hog.h"

// <cmd> {<len> <column id>} <type> <#kvs> [{<len> <key>} {<len> <value>}]...
void hog_put(server_t *s, grn_ctx *ctx)
{
    uint32_t len;
    receive(s->socket, &len, sizeof(len));
    len = ntohl(len);
    char *buf = malloc(len);
    receive(s->socket, buf, len);
    grn_obj *col = grn_ctx_get(ctx, buf, len);
    grn_obj *table = grn_column_table(ctx, col);
    // get key and value types
    char type;
    receive(s->socket, &type, 1);
    // submit values for each keys
    uint32_t nkeys;
    receive(s->socket, &nkeys, sizeof(nkeys));
    nkeys = ntohl(nkeys);
    for(uint32_t i = 0; i < nkeys; ++i){
        receive(s->socket, &len, sizeof(len));
        len = ntohl(len);
        buf = realloc(buf, len);
        receive(s->socket, buf, len);
        ntoh_buf(buf, len, type);
        grn_id id = grn_table_add(ctx, table, buf, len, NULL);
        receive(s->socket, &len, sizeof(len));
        len = ntohl(len);
        buf = realloc(buf, len);
        receive(s->socket, buf, len);
        ntoh_buf(buf, len, type);
        grn_obj value;
        GRN_OBJ_INIT(&value, GRN_BULK, 0, type);
        grn_bulk_write(ctx, &value, buf, len);
        grn_obj_set_value(ctx, col, id, &value, GRN_OBJ_SET);
    }
cleanup:
    free(buf);
    GRN_OBJ_FIN(ctx, table);
    GRN_OBJ_FIN(ctx, col);
}
