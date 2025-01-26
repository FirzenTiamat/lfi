#include "cflowtracer.h"


static CFBuffer* cfbuf = NULL;

static inline void flush_cfbuf(){
    cfbuf = (CFBuffer*) syscall(499);  //in liblfix/syscall.h and syscall.c of arm64
    if (cfbuf == NULL || cfbuf == (void*)-1){
        exit(-1);
    }
    if (cfbuf->size <=0 || cfbuf->nextpos != 0){  //cfbuf->size
        exit(-2);
    }
    return;
}

// just get the pointer of cfbuf,
// should be instrumented at the entry of the program.
// avoid checking every time in running.
void init_cfbuf(){
    flush_cfbuf();
    return;
}

// just flush the cfbuf,
// could be instrumented at the exit of the program but may the lfi runtime will do that.
// 2 similar functions with different names for instrumentation and optimization
void pure_cfbuf(){
    flush_cfbuf();
    return;
}

static inline void cflow_logger(void* dst, void* src){
    // if (cfbuf == NULL) flush_cfbuf();
    cfbuf->cflogs[cfbuf->nextpos] = (CFLog){(uint32_t)dst, (uint32_t)src};
    // cfbuf->cflogs[cfbuf->nextpos].srcaddr = (uint32_t)src;
    // cfbuf->cflogs[cfbuf->nextpos].dstaddr = (uint32_t)dst;
    cfbuf->nextpos++;
    if (cfbuf->nextpos >= cfbuf->size){
        flush_cfbuf();
    }
    return;
}

// 3 similar trace functions with different names for instrumentation and optimization
void tracecall(void* dest_to){
    void *from = __builtin_return_address(0);
    cflow_logger(dest_to, from);
    return;
}

void traceret(void* dest_to){
    void *from = __builtin_return_address(0);
    cflow_logger(dest_to, from);
    return;
}

void traceindirectbr(void* dest_to){
    void *from = __builtin_return_address(0);
    cflow_logger(dest_to, from);
    return;
}