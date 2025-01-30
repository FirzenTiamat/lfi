#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hiredis/hiredis.h"
#include "lfix.h"

#define REDIS_SERVER_IP "127.0.0.1"
#define REDIS_SERVER_PORT 6379
#define REDIS_SERVER_TIMEOUT 500000  // 0.5 second
#define KEY_PREFIX "cflogs"

static size_t
store_cfbuf(LFIXProc* p)
{
    const CFBuffer* cfbuf = p->cfbuf;
    // printf("store_cfbuf: p->base: %p\n", p->base);
    if (cfbuf == NULL || cfbuf->size == 0){
        fprintf(stderr, "CFBuffer is null or empty\n");
        return 0;
    }
    if (cfbuf->nextpos > cfbuf->size) {
        fprintf(stderr, "CFBuffer overflow: %lu\n", cfbuf->nextpos);
        return 0;
    }
    redisContext *c = redisConnect(REDIS_SERVER_IP, REDIS_SERVER_PORT);
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
        // reply = (redisReply*)redisCommand(c, "RPUSH %s%p %p %p", KEY_PREFIX, p->base, cfbuf->cflogs[i].dstaddr, cfbuf->cflogs[i].srcaddr);

        // in cflowtracer, the dstaddr and srcaddr are truncted to 32 bits, so theoretically we need to subtract the truncted base address
        // however, the p->base should always be a multiple of 4GB, so the lower 32 bits should be 0
        // reply = (redisReply*)redisCommand(c, "RPUSH %s %x %x", KEY_PREFIX, cfbuf->cflogs[i].dstaddr - (uint32_t)p->base, cfbuf->cflogs[i].srcaddr - (uint32_t)p->base);
        reply = (redisReply*)redisCommand(c, "RPUSH %s %x %x", KEY_PREFIX, cfbuf->cflogs[i].dstaddr, cfbuf->cflogs[i].srcaddr);
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