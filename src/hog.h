#ifndef __HOG_H__
#define __HOG_H__
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <groonga/groonga.h>

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

#endif
