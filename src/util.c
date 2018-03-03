#include "hog.h"
#define MAX_RECEIVE   (64*1024*1024)

int submit(int s, const void *buf, ssize_t len)
{
    while(len > 0){
        int ret = send(s, buf, len, 0);
        if(ret == 0) return EBADF;
        else if(ret < 0) return ret;
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

int submit_one(int s)
{
    const static char one = 1;
    return submit(s, &one, 1);
}

int receive(int s, void *buf, ssize_t len)
{
    if(len > MAX_RECEIVE) return ENOBUFS;
    while(len > 0){
        int ret = recv(s, buf, len, 0);
        if(ret == 0) return EBADF;
        else if(ret < 0) return ret;
        buf += ret;
        len -= ret;
    }
    return 0;
}

void ntoh_buf(void *buf, uint32_t len, char type)
{
    switch(type){
    case GRN_DB_INT16: case GRN_DB_UINT16:
        *(uint16_t*)buf = ntohs(*(uint16_t*)buf);
        break;
    case GRN_DB_INT32: case GRN_DB_UINT32:
        *(uint32_t*)buf = ntohl(*(uint32_t*)buf);
        break;
    }
}

void hton_buf(void *buf, uint32_t len, char type)
{
    switch(type){
    case GRN_DB_INT16: case GRN_DB_UINT16:
        *(uint16_t*)buf = htons(*(uint16_t*)buf);
        break;
    case GRN_DB_INT32: case GRN_DB_UINT32:
        *(uint32_t*)buf = htonl(*(uint32_t*)buf);
        break;
    }
}

void *hog_alloc(void *buf, ssize_t len)
{
    void *next = realloc(buf, len);
    if(next == NULL){
        if(len == 0) return next;
        fprintf(stderr, "Failed to realloc %ld bytes.\n", len);
        free(buf);
        next = malloc(len);
        if(next == NULL){
            fprintf(stderr, "Failed to malloc %ld bytes.\n", len);
        }
    }
    return next;
}
