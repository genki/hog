#include "hog.h"

// Find keys that records have specified values 
// <cmd> {<len> <column id>} <key type> {<len> <query>}
// {<len> <sort keys>} <offset> <limit>
void hog_query(server_t *s, grn_ctx *ctx)
{
    uint32_t len;
    HOG_RECV(s, &len, sizeof(len), return);
    len = ntohl(len);
    char *buf = hog_alloc(NULL, len);
    HOG_RECV(s, buf, len, goto cleanup);
    grn_obj *table, *col;
    col = grn_ctx_get(ctx, buf, len);
    if(grn_obj_is_table(ctx, col)) table = col;
    else table = grn_column_table(ctx, col);
    // get key type
    char type;
    HOG_RECV(s, &type, 1, goto cleanup);
    // get query and make expr
    HOG_RECV(s, &len, sizeof(len), goto cleanup);
    len = ntohl(len);
    buf = hog_alloc(buf, len);
    HOG_RECV(s, buf, len, goto cleanup);
    grn_obj *query, *var;
    GRN_EXPR_CREATE_FOR_QUERY(ctx, table, query, var);
    grn_expr_flags flags = GRN_EXPR_SYNTAX_QUERY|GRN_EXPR_ALLOW_LEADING_NOT|
        GRN_EXPR_ALLOW_COLUMN|GRN_EXPR_ALLOW_PRAGMA;
    grn_expr_append_obj(ctx, query, col, GRN_OP_PUSH, 1);
    grn_rc rc = grn_expr_parse(ctx, query, buf, len, col,
            GRN_OP_MATCH, GRN_OP_AND, flags); 
    // get sort keys
    HOG_RECV(s, &len, sizeof(len), goto query_fin);
    len = ntohl(len);
    if(len > 0){
        buf = hog_alloc(buf, len);
        HOG_RECV(s, buf, len, goto query_fin);
    }
    int32_t offset, limit;
    HOG_RECV(s, &offset, sizeof(offset), goto query_fin);
    offset = ntohl(offset);
    HOG_RECV(s, &limit, sizeof(limit), goto query_fin);
    limit = ntohl(limit);
    // check
    if(rc != GRN_SUCCESS) {
        fprintf(stderr, "Failed to parse query: %s\n", ctx->errbuf);
        uint32_t zero = htonl(0);
        HOG_SEND(s, &zero, sizeof(zero), goto query_fin);
        goto query_fin;
    }
    // perform query
    grn_obj *result = grn_table_select(ctx, table, query, NULL, GRN_OP_OR);
    uint32_t total = grn_table_size(ctx, result);
    total = htonl(total);
    HOG_SEND(s, &total, sizeof(total), goto result_fin);
    if(total == 0){
        HOG_SEND(s, &total, sizeof(total), goto result_fin);
        goto result_fin;
    }
    uint32_t nsorters = 0;
    grn_table_sort_key *sorters = NULL;
    sorters = grn_table_sort_key_from_str(ctx, buf, len, result, &nsorters);
    grn_obj *sorted = grn_table_create(ctx, NULL, 0, NULL,
            GRN_OBJ_TABLE_NO_KEY, NULL, result);
    grn_table_sort(ctx, result, offset, limit, sorted, sorters, nsorters);
    uint32_t count = grn_table_size(ctx, sorted);
    count = htonl(count);
    HOG_SEND(s, &count, sizeof(count), goto sorted_fin);
    grn_table_cursor *cursor = grn_table_cursor_open(ctx, sorted,
            NULL, 0, NULL, 0, 0, -1, GRN_CURSOR_BY_ID); 
    grn_obj *_key = grn_obj_column(ctx, result,
            GRN_COLUMN_NAME_KEY, GRN_COLUMN_NAME_KEY_LEN);
    grn_obj value;
    GRN_RECORD_INIT(&value, 0, grn_obj_get_range(ctx, result));
    while(grn_table_cursor_next(ctx, cursor) != GRN_ID_NIL){
        void *id;
        grn_table_cursor_get_value(ctx, cursor, &id);
        GRN_BULK_REWIND(&value);
        grn_obj_get_value(ctx, _key, *(grn_id*)id, &value);
        void *bulk = GRN_BULK_HEAD(&value);
        uint32_t blen = GRN_BULK_VSIZE(&value);
        uint32_t nblen = htonl(blen);
        HOG_SEND(s, &nblen, sizeof(nblen), break);
        hton_buf(bulk, blen, type);
        HOG_SEND(s, bulk, blen, break);
    }
key_fin:
    GRN_OBJ_FIN(ctx, _key);
value_fin:
    GRN_OBJ_FIN(ctx, &value);
cursor_fin:
    grn_table_cursor_close(ctx, cursor);
sorted_fin:
    GRN_OBJ_FIN(ctx, sorted);
sorter_fin:
    if(nsorters) grn_table_sort_key_close(ctx, sorters, nsorters);
result_fin:
    GRN_OBJ_FIN(ctx, result);
query_fin:
    GRN_OBJ_FIN(ctx, query);
    GRN_OBJ_FIN(ctx, var);
cleanup:
    free(buf);
}
