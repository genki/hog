#ifndef __HOG_H__
#define __HOG_H__
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

typedef struct {
    const char *db_path;
    const char *bind;
    int port;
    int num_threads;
    int max_conn;
    int socket;
} hog_t;

typedef struct {
    hog_t *hog;
    int socket;
} server_t;

void* server(void *arg);
int submit(int s, const void *buf, ssize_t len);
int submit_chunk(int s, const char *buf);
int receive(int s, void *buf, ssize_t len);
void ntoh_buf(void *buf, uint32_t len, char type);
void hton_buf(void *buf, uint32_t len, char type);

void hog_ping(server_t *s, grn_ctx *ctx);
void hog_get(server_t *s, grn_ctx *ctx);
void hog_put(server_t *s, grn_ctx *ctx);
void hog_del(server_t *s, grn_ctx *ctx);
void hog_exist(server_t *s, grn_ctx *ctx);
void hog_fin(server_t *s, grn_ctx *ctx);
void hog_count(server_t *s, grn_ctx *ctx);

#endif
