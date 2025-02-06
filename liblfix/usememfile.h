/**
 * file path: /tmp/att-sfi/cflogs
 * type: memory file using tmpfs
 * usage: store_cfbuf
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>

#include "lfix.h"

#define FILE_CFLOGS_PATH "/tmp/att-sfi/cflogs"

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
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_END;
    lock.l_start = 0;
    lock.l_len = sizeof(CFLog) * cfbuf->nextpos;
    
    int fd = open(FILE_CFLOGS_PATH, O_WRONLY | O_APPEND, 0666);
    if (fd < 0) {
        fprintf(stderr, "Error opening file %s\n", FILE_CFLOGS_PATH);
        return 0;
    }
    if (fcntl(fd, F_SETLKW, &lock) < 0) {
        fprintf(stderr, "Error locking file %s\n", FILE_CFLOGS_PATH);
        close(fd);
        return 0;
    }
    if (write(fd, cfbuf->cflogs, sizeof(CFLog) * cfbuf->nextpos) < 0) {
        fprintf(stderr, "Error writing file %s\n", FILE_CFLOGS_PATH);
        close(fd);
        return 0;
    }
    // for (size_t i = 0; i < cfbuf->nextpos; i++) {
    //     char buf[20]; // larger than 8 + 1 + 8 + 1 + 1
    //     sprintf(buf, "%x %x\n", cfbuf->cflogs[i].srcaddr, cfbuf->cflogs[i].dstaddr);
    //     if (write(fd, buf, strlen(buf)) < 0) {
    //         fprintf(stderr, "Error writing file %s\n", FILE_CFLOGS_PATH);
    //         close(fd);
    //         return i;
    //     }
    // }
    lock.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLK, &lock) < 0) {
        fprintf(stderr, "Error unlocking file %s\n", FILE_CFLOGS_PATH);
        close(fd);
        return 0;
    }
    close(fd);
    return cfbuf->nextpos;
}