#include "hog.h"

// Find keys that records have specified values 
// <cmd> {<len> <column id>} <key type> {<len> <query>}
void hog_query(server_t *s, grn_ctx *ctx)
{
    uint32_t len;
    HOG_RECV(s, &len, sizeof(len), return);
    len = ntohl(len);
    char *buf = hog_alloc(NULL, len);
    HOG_RECV(s, buf, len, goto cleanup);
    grn_obj *col = grn_ctx_get(ctx, buf, len);
    grn_obj *table = grn_column_table(ctx, col);
    // get key type
    char type;
    HOG_RECV(s, &type, 1, goto cleanup);
    // get query and submit keys
    HOG_RECV(s, &len, sizeof(len), goto cleanup);
    len = ntohl(len);
    buf = hog_alloc(buf, len);
    HOG_RECV(s, buf, len, goto cleanup);

    grn_obj *query, *var;
    GRN_EXPR_CREATE_FOR_QUERY(ctx, table, query, var);
    grn_expr_flags flags = GRN_EXPR_SYNTAX_QUERY|GRN_EXPR_ALLOW_LEADING_NOT;
    grn_expr_append_obj(ctx, query, col, GRN_OP_PUSH, 1);
    grn_rc rc = grn_expr_parse(ctx, query, buf, len, col,
            GRN_OP_MATCH, GRN_OP_AND, flags); 
    if(rc != GRN_SUCCESS) {
        GRN_OBJ_FIN(ctx, query);
        GRN_OBJ_FIN(ctx, var);
        fprintf(stderr, "Failed to parse query: %s\n", ctx->errbuf);
        uint32_t zero = htonl(0);
        HOG_SEND(s, &zero, sizeof(zero), goto cleanup);
        goto cleanup;
    }
    grn_obj *result = grn_table_select(ctx, table, query, NULL, GRN_OP_OR);
    GRN_OBJ_FIN(ctx, query);
    GRN_OBJ_FIN(ctx, var);
    uint32_t count = grn_table_size(ctx, result);
    count = htonl(count);
    HOG_SEND(s, &count, sizeof(count), goto result_fin);
    if(count == 0) goto result_fin;
    grn_table_cursor *cursor = grn_table_cursor_open(ctx, result,
            NULL, 0, NULL, 0, 0, 1, 0); 
    buf = hog_alloc(buf, GRN_TABLE_MAX_KEY_SIZE);
    while(grn_table_cursor_next(ctx, cursor) != GRN_ID_NIL){
        void *key;
        grn_table_cursor_get_key(ctx, cursor, &key);
        buf = hog_alloc(buf, GRN_TABLE_MAX_KEY_SIZE);
        len = grn_table_get_key(ctx, table, *(grn_id*)key,
                buf, GRN_TABLE_MAX_KEY_SIZE);
        hton_buf(buf, len, type);
        uint32_t nlen = htonl(len);
        HOG_SEND(s, &nlen, sizeof(nlen), goto cursor_fin);
        HOG_SEND(s, buf, len, goto cursor_fin);
    }
cursor_fin:
    grn_table_cursor_close(ctx, cursor);
result_fin:
    GRN_OBJ_FIN(ctx, result);
cleanup:
    free(buf);
}
