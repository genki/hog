#ifndef __HOG_H__
#define __HOG_H__
#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <groonga.h>

#define HOG_RECV(s, buf, len, fail) \
    if(((s)->error = receive((s)->socket, (buf), (len))) != 0){ \
        fprintf(stderr, "Failed to recv %u bytes to %p (%s:%d) %d\n", \
                (uint32_t)len, buf, __FILE__, __LINE__, (s)->error); \
        fail; \
    }
#define HOG_SEND(s, buf, len, fail) \
    if(((s)->error = submit((s)->socket, (buf), (len))) != 0){ \
        fprintf(stderr, "Failed to send %u bytes from %p (%s:%d) %d\n", \
                (uint32_t)len, buf, __FILE__, __LINE__, (s)->error); \
        fail; \
    }

struct server_t;

typedef struct {
    int socket;
    int verbose;
    struct server_t **servers;
    pthread_t *threads;
    pthread_mutex_t mutex;
    int nservers;
    grn_obj *db;
    grn_ctx *ctx;
} hog_t;

typedef struct server_t {
    hog_t *hog;
    int socket;
    int thread_id;
    volatile int running;
    volatile int killed;
    volatile int error;
} server_t;

void* server(void *arg);
int submit(int s, const void *buf, ssize_t len);
int submit_chunk(int s, const char *buf);
int submit_one(int s);
int receive(int s, void *buf, ssize_t len);
void ntoh_buf(void *buf, uint32_t len, char type);
void hton_buf(void *buf, uint32_t len, char type);
void* hog_alloc(void *buf, ssize_t len);

void hog_ping(server_t *s, grn_ctx *ctx);
void hog_get(server_t *s, grn_ctx *ctx);
void hog_put(server_t *s, grn_ctx *ctx);
void hog_set(server_t *s, grn_ctx *ctx);
void hog_del(server_t *s, grn_ctx *ctx);
void hog_exist(server_t *s, grn_ctx *ctx);
void hog_fin(server_t *s, grn_ctx *ctx);
void hog_count(server_t *s, grn_ctx *ctx);
void hog_fetch(server_t *s, grn_ctx *ctx);
void hog_store(server_t *s, grn_ctx *ctx);
void hog_find(server_t *s, grn_ctx *ctx);
void hog_query(server_t *s, grn_ctx *ctx);
void hog_each(server_t *s, grn_ctx *ctx);

#endif
