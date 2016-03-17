#include "hog.h"

// Find keys that records have specified values 
// <cmd> {<len> <column id>} <key type> <value type>
// <#values> [<len> <value>]...
void hog_find(server_t *s, grn_ctx *ctx)
{
    uint32_t len;
    HOG_RECV(s, &len, sizeof(len), return);
    len = ntohl(len);
    char *buf = malloc(len);
    HOG_RECV(s, buf, len, goto cleanup);
    grn_obj *col = grn_ctx_get(ctx, buf, len);
    grn_obj *table = grn_column_table(ctx, col);
    // get key and value types
    char types[2];
    HOG_RECV(s, types, 2, goto cleanup);
    // get values and submit keys
    uint32_t nvalues;
    HOG_RECV(s, &nvalues, sizeof(nvalues), goto cleanup);
    nvalues = ntohl(nvalues);
    grn_obj value;
    GRN_OBJ_INIT(&value, GRN_BULK, 0, types[1]);
    for(uint32_t i = 0; i < nvalues; ++i){
        HOG_RECV(s, &len, sizeof(len), goto value_fin);
        len = ntohl(len);
        buf = realloc(buf, len);
        HOG_RECV(s, buf, len, goto value_fin);
        if(col == NULL){
            uint32_t zero = htonl(0);
            HOG_SEND(s, &zero, sizeof(zero), goto value_fin);
            continue;
        }
        ntoh_buf(buf, len, types[1]);
        GRN_BULK_REWIND(&value);
        grn_bulk_write(ctx, &value, buf, len);
        grn_obj *query, *var;
        GRN_EXPR_CREATE_FOR_QUERY(ctx, table, query, var);
        grn_expr_append_obj(ctx, query, var, GRN_OP_PUSH, 1);
        grn_expr_append_const(ctx, query, col, GRN_OP_PUSH, 1);
        grn_expr_append_op(ctx, query, GRN_OP_GET_VALUE, 2);
        grn_expr_append_const(ctx, query, &value, GRN_OP_PUSH, 1);
        grn_expr_append_op(ctx, query, GRN_OP_EQUAL, 2);
        grn_obj *result = grn_table_select(ctx, table, query, NULL, GRN_OP_OR);
        GRN_OBJ_FIN(ctx, query);
        GRN_OBJ_FIN(ctx, var);
        grn_table_cursor *cursor = grn_table_cursor_open(ctx, result,
                NULL, 0, NULL, 0, 0, 1, 0); 
        if(cursor && grn_table_cursor_next(ctx, cursor) != GRN_ID_NIL){
            void *key;
            grn_table_cursor_get_key(ctx, cursor, &key);
            buf = realloc(buf, GRN_TABLE_MAX_KEY_SIZE);
            len = grn_table_get_key(ctx, table, *(grn_id*)key,
                    buf, GRN_TABLE_MAX_KEY_SIZE);
            hton_buf(buf, len, types[0]);
            uint32_t nlen = htonl(len);
            HOG_SEND(s, &nlen, sizeof(nlen), goto value_fin);
            HOG_SEND(s, buf, len, goto value_fin);
        }else{
            uint32_t zero = htonl(0);
            HOG_SEND(s, &zero, sizeof(zero), goto value_fin);
        }
        if(cursor) grn_table_cursor_close(ctx, cursor);
        GRN_OBJ_FIN(ctx, result);
    }
value_fin:
    GRN_OBJ_FIN(ctx, &value);
cleanup:
    free(buf);
}
