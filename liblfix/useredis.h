#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hiredis/hiredis.h"
#include "lfix.h"

#define REDIS_SERVER_IP "127.0.0.1"
#define REDIS_SERVER_PORT 6379
#define REDIS_SERVER_TIMEOUT 1000000  // 1 second
#define KEY_PREFIX "cflogs:"

static size_t
store_cfbuf(LFIXProc* p)
{
    const CFBuffer* cfbuf = p->cfbuf;
    if (cfbuf == NULL || cfbuf->size == 0){
        fprintf(stderr, "CFBuffer is null or empty\n");
        return 0;
    }
    if (cfbuf->nextpos > cfbuf->size) {
        fprintf(stderr, "CFBuffer overflow: %lu\n", cfbuf->nextpos);
        return 0;
    }
    redisContext *c = redisConnectWithTimeout(REDIS_SERVER_IP, REDIS_SERVER_PORT, (struct timeval) {0, REDIS_SERVER_TIMEOUT});
    redisReply *reply;
    if (c == NULL || c->err) {
        if (c) {
            fprintf(stderr, "Connection error: %s\n", c->errstr);
            redisFree(c);
        } else {
            fprintf(stderr, "Connection error: can't allocate redis context\n");
        }
        return 0;
    }
    for (size_t i = 0; i < cfbuf->nextpos; i++) {
        reply = (redisReply*)redisCommand(c, "RPUSH %s%p %p %p", KEY_PREFIX, p->base, cfbuf->cflogs[i].dstaddr, cfbuf->cflogs[i].srcaddr);
        if (reply == NULL) {
            fprintf(stderr, "RPUSH Error: %s\n", c->errstr);
            redisFree(c);
            return i;
        }
        freeReplyObject(reply);
    }
    redisFree(c);
    return cfbuf->nextpos;
}