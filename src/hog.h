#ifndef __HOG_H__
#define __HOG_H__
#include <groonga/groonga.h>

typedef struct {
    const char *db_path;
    const char *bind;
    int port;
    int num_threads;
    int max_conn;
    grn_ctx ctx;
    grn_obj *db;
    int socket;
} hog_t;

#endif
