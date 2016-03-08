#include "hog.h"
#define MAX_CMD_NAME 16

hog_t hog;

typedef void (*CMD_HANDLER)(server_t*, grn_ctx*);
void hog_get(server_t *s, grn_ctx *ctx);
void hog_put(server_t *s, grn_ctx *ctx);

static struct {
    char name[MAX_CMD_NAME];
    CMD_HANDLER handler;
} cmd_handlers[] = {
    {"GET", hog_get},
    {"PUT", hog_put},
};

int submit(int s, const void *buf, ssize_t len)
{
    while(len > 0){
        int ret = send(s, buf, len, 0);
        if(ret < 0) return ret;
        buf += ret;
        len -= ret;
    }
    return 0;
}

int submit_chunk(int s, const char *buf)
{
    uint32_t len = strlen(buf);
    uint32_t nlen = htonl(len);
    if(submit(s, &nlen, sizeof(nlen)) != 0) return -1;
    if(submit(s, buf, len) != 0) return -1;
    return 0;
}

int receive(int s, void *buf, ssize_t len)
{
    while(len > 0){
        int ret = recv(s, buf, len, 0);
        if(ret < 0) return ret;
        buf += ret;
        len -= ret;
    }
    return 0;
}

void ntoh_buf(void *buf, uint32_t len, char type){
    switch(type){
    case GRN_DB_INT16: case GRN_DB_UINT16:
        *(uint16_t*)buf = ntohs(*(uint16_t*)buf);
        break;
    case GRN_DB_INT32: case GRN_DB_UINT32:
        *(uint32_t*)buf = ntohl(*(uint32_t*)buf);
        break;
    }
}

void hton_buf(void *buf, uint32_t len, char type){
    switch(type){
    case GRN_DB_INT16: case GRN_DB_UINT16:
        *(uint16_t*)buf = htons(*(uint16_t*)buf);
        break;
    case GRN_DB_INT32: case GRN_DB_UINT32:
        *(uint32_t*)buf = htonl(*(uint32_t*)buf);
        break;
    }
}

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

void* server(void *arg){
    int loop = 1;
    char num_handlers = sizeof(cmd_handlers)/sizeof(cmd_handlers[0]);
    server_t *s = (server_t*)arg;
    hog_t *hog = s->hog;
    grn_ctx ctx;
    grn_ctx_init(&ctx, 0);
    // open db
    grn_obj *db = grn_db_open(&ctx, hog->db_path);
    // setup socket
    struct timeval to;
    to.tv_sec = 10;
    to.tv_usec = 0;
    setsockopt(s->socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&to, sizeof(to));
    setsockopt(s->socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&to, sizeof(to));
    // send the command list
    if(submit(s->socket, &num_handlers, 1) != 0){
        fprintf(stderr, "Failed to send num handlers\n");
    }
    for(char i = 0; i < num_handlers; ++i){
        const char *name = cmd_handlers[i].name;
        if(submit_chunk(s->socket, name) != 0){
            fprintf(stderr, "Failed to send cmd (%d) %s\n", i, name);
        }
    }
    while(loop){
        char cmd;
        if(receive(s->socket, &cmd, 1) != 0){
            fprintf(stderr, "Failed to recv cmd.\n");
            loop = 0;
            break;
        }
        if(cmd < num_handlers) (cmd_handlers[cmd].handler)(s, &ctx);
        else fprintf(stderr, "Invalid cmd: %d\n", cmd);
    }
cleanup:
    GRN_OBJ_FIN(&ctx, db);
    grn_ctx_fin(&ctx);
    close(s->socket);
    free(s);
    return NULL;
}
