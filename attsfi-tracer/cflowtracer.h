#ifndef _MEASURE_AGENT_H
#define _MEASURE_AGENT_H

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

// have to be same as the defination in liblfix/lfix.h
typedef struct {
    uint64_t dstaddr : 32;  //lower 32 bits
    uint64_t srcaddr : 32;  //higher 32 bits
} CFLog;

typedef struct {
    size_t size;
    size_t nextpos;
    CFLog* cflogs;
} CFBuffer;

void init_cfbuf();
void pure_cfbuf();
void __attribute__((noinline)) tracecall(void* dest_to);
void __attribute__((noinline)) traceret(void* dest_to);
void __attribute__((noinline)) traceindirectbr(void* dest_to);

#endif