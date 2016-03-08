#include "hog.h"

// <cmd> {<len> <column id>} <type> <#keys> [{<len> <key>}]...
void hog_exist(server_t *s, grn_ctx *ctx)
{
    uint32_t len;
    receive(s->socket, &len, sizeof(len));
    len = ntohl(len);
    char *buf = malloc(len);
    receive(s->socket, buf, len);
    grn_obj *col = grn_ctx_get(ctx, buf, len);
    grn_obj *table = grn_column_table(ctx, col);
    // get key type
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
        grn_id id = grn_table_get(ctx, table, buf, len);
        char flag = (id == GRN_ID_NIL ? 0 : 1);
        submit(s->socket, &flag, 1);
    }
cleanup:
    free(buf);
    GRN_OBJ_FIN(ctx, table);
    GRN_OBJ_FIN(ctx, col);
}
