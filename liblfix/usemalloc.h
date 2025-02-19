#pragma once
/**
 * use malloc to store the cflogs,
 * and in main.c when the program exits, dump the cflogs to a file once and for all.
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>

#include "lfix.h"

// This file records the cflogs in a memory file using tmpfs.
// if the file is not found, this step will be skipped to measure
// the performance of the program without the overhead of storing.
#define FILE_CFLOGS_PATH "/tmp/att-sfi/cflogs"

// only use same memory to dump cfbuf repeatedly for test
#ifndef SHINGLED_CFBUF
#define SHINGLED_CFBUF 1
#endif

typedef struct CFBufferChainNode {
    CFBuffer* cfbuf;
    struct CFBufferChainNode* next;
} CFBufferChainNode;

__attribute__((weak)) CFBufferChainNode* cfbufchainhead = NULL;
__attribute__((weak)) CFBufferChainNode* cfbufchaintail = NULL;

static size_t
store_cfbuf(LFIXProc* p)
{
    const CFBuffer* pcfbuf = p->cfbuf;

    size_t cfbuffsize;
    if (SHINGLED_CFBUF == 0) {
        cfbuffsize = pcfbuf->nextpos * sizeof(CFLog) + sizeof(CFBuffer); //must no more than CFBUFFSIZE defined in proc.h;
    }
    else {
        cfbuffsize = pcfbuf->size * sizeof(CFLog) + sizeof(CFBuffer); // if use shingled cfbuf, the size of cfbuf must be the MAX size
    }
    if (pcfbuf == NULL || pcfbuf->size == 0){
        fprintf(stderr, "CFBuffer is null or empty\n");
        return 0;
    }
    if (pcfbuf->nextpos > pcfbuf->size) {
        fprintf(stderr, "CFBuffer overflow: %lu\n", pcfbuf->nextpos);
        return 0;
    }
    // chain init check
    if (cfbufchainhead == NULL) {
        // fprintf(stderr, "Starting init chain\n");
        cfbufchainhead = (CFBufferChainNode*) malloc(sizeof(CFBufferChainNode));
        if (cfbufchainhead == NULL) {
            fprintf(stderr, "Error allocating memory for CFBufferChain head\n");
            return 0;
        }
        cfbufchainhead->cfbuf = NULL;  //cfbuf in head node is always empty
        cfbufchaintail = (CFBufferChainNode*) malloc(sizeof(CFBufferChainNode));
        if (cfbufchaintail == NULL) {
            fprintf(stderr, "Error allocating memory for CFBufferChain tail\n");
            return 0;
        }
        cfbufchaintail->cfbuf = NULL;  //cfbuf in tail node is always empty
        cfbufchaintail->next = NULL;
        cfbufchainhead->next = cfbufchaintail;
    }
    // chain append
    if (SHINGLED_CFBUF == 0 || cfbufchaintail->cfbuf == NULL) { 
        cfbufchaintail->cfbuf = (CFBuffer*) malloc(cfbuffsize);
    } // else it uses the pre node's cfbuf

    if (cfbufchaintail->cfbuf == NULL) {
        fprintf(stderr, "Error allocating memory for CFBuffer in CFBufferChain\n");
        return 0;
    }
    memcpy(cfbufchaintail->cfbuf, pcfbuf, cfbuffsize);

    // chain tail reset
    cfbufchaintail->next = (CFBufferChainNode*) malloc(sizeof(CFBufferChainNode));
    if (cfbufchaintail->next == NULL) {
        fprintf(stderr, "Error allocating memory for next CFBufferChainNode\n");
        return 0;
    }
    cfbufchaintail->next->cfbuf = cfbufchaintail->cfbuf; // shingled cfbuf use
    cfbufchaintail->next->next = NULL;
    cfbufchaintail = cfbufchaintail->next;

    // return the number of cflogs stored
    // printf("store_cfbuf: %lu cflogs stored\n", pcfbuf->nextpos);
    return pcfbuf->nextpos;
}

static size_t 
dump_cfbuf_chain(size_t *nodenum)
{
    // fprintf(stderr, "Starting dump_cfbuf_chain\n");
    if (cfbufchainhead == NULL || cfbufchaintail == NULL) {
        fprintf(stderr, "CFBufferChain is not found\n");
        return 0;
    }
    if (cfbufchainhead->next == NULL) {
        fprintf(stderr, "CFBufferChain is empty\n");
        return 0;
    }
    size_t cflog_count = 0;
    size_t node_count = 0;

    int fd = open(FILE_CFLOGS_PATH, O_WRONLY | O_APPEND, 0666);
    if (fd < 0) {
        return 0;
    }
 
    for (CFBufferChainNode* node = cfbufchainhead->next; node != cfbufchaintail; node = node->next) {
        if (node->cfbuf == NULL || node->cfbuf->size == 0) {
            fprintf(stderr, "CFBuffer in CFBufferChainNode %ld is empty\n", node_count);
            continue;
        }
        if (node->cfbuf->nextpos > node->cfbuf->size) {
            fprintf(stderr, "CFBuffer in CFBufferChainNode %ld overflow: %lu\n", node_count, node->cfbuf->nextpos);
            continue;
        }
        
        cflog_count += node->cfbuf->nextpos;
        node_count++;
        if (SHINGLED_CFBUF == 0) {
            if (write(fd, node->cfbuf->cflogs, sizeof(CFLog) * node->cfbuf->nextpos) < 0) {
                fprintf(stderr, "Error writing file %s\n", FILE_CFLOGS_PATH);
                close(fd);
                return 0;
            }
            free(node->cfbuf);  // cannot free if use shingled cfbuf
        }
        else{  // only write last node
            if (node->next != cfbufchaintail) {
                continue;
            }
            if (write(fd, node->cfbuf->cflogs, sizeof(CFLog) * node->cfbuf->nextpos) < 0) {
                fprintf(stderr, "Error writing file %s\n", FILE_CFLOGS_PATH);
                close(fd);
                return 0;
            }
        }
    }
    close(fd);
    *nodenum = node_count;
    return cflog_count;
}