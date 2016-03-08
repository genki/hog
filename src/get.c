#include "hog.h"

// <cmd> {<len> <column id>} <types> <#keys> [{<len> <key>}]...
void hog_get(server_t *s, grn_ctx *ctx)
{
    uint32_t len;
    receive(s->socket, &len, sizeof(len));
    len = ntohl(len);
    char *buf = malloc(len);
    receive(s->socket, buf, len);
    grn_obj *col = grn_ctx_get(ctx, buf, len);
    grn_obj *table = grn_column_table(ctx, col);
    // get key and value types
    char types[2];
    receive(s->socket, types, 2);
    // submit values for each keys
    uint32_t nkeys;
    receive(s->socket, &nkeys, sizeof(nkeys));
    nkeys = ntohl(nkeys);
    for(uint32_t i = 0; i < nkeys; ++i){
        receive(s->socket, &len, sizeof(len));
        len = ntohl(len);
        buf = realloc(buf, len);
        receive(s->socket, buf, len);
        ntoh_buf(buf, len, types[0]);
        grn_id id = grn_table_get(ctx, table, buf, len);
        grn_obj *value = grn_obj_get_value(ctx, col, id, NULL);
        if(value->header.type == GRN_BULK){
            void *bulk = GRN_BULK_HEAD(value);
            uint32_t blen = GRN_BULK_VSIZE(value);
            uint32_t nblen = htonl(blen);
            submit(s->socket, &nblen, sizeof(nblen));
            hton_buf(bulk, blen, types[1]);
            submit(s->socket, bulk, blen);
        }else{
            char zero = 0;
            submit(s->socket, &zero, 1);
        }
        GRN_OBJ_FIN(ctx, value);
    }
cleanup:
    free(buf);
    GRN_OBJ_FIN(ctx, table);
    GRN_OBJ_FIN(ctx, col);
}
