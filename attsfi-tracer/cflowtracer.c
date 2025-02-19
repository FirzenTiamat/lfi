#include "cflowtracer.h"
// #include <stdio.h>

static CFBuffer* cfbuf = NULL;

static inline void flush_cfbuf(){
    cfbuf = (CFBuffer*) syscall(499);  //in liblfix/syscall.h and syscall.c of arm64
    // printf("cfbuf: %p, cfbuf.size: %zu, cfbuf.nextpos: %zu\n", cfbuf, cfbuf->size, cfbuf->nextpos);
    // if (cfbuf == NULL || cfbuf == (void*)-1){
    //     exit(-1);
    // }
    // if (cfbuf->size <=0 || cfbuf->nextpos != 0){  //cfbuf->size
    //     exit(-2);
    // }
    return;
}

// just get the pointer of cfbuf,
// should be instrumented at the entry of the program.
// avoid checking every time in running.
void init_cfbuf(){
    // printf("init_cfbuf\n");
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
    // if (cfbuf == NULL)    {
    // //     fprintf(stderr, "cflow_logger: cfbuf is NULL\n");
    // //     exit(0);
    //     flush_cfbuf();
    //     // fprintf(stderr, "cflow_logger: cfbuf is NULL\n");
    //     // if (cfbuf == NULL){
    //     //     fprintf(stderr, "cflow_logger: cannot alloc cfbuf\n");
    //     //     exit(0);
    //     // }
    // }
    if (cfbuf != NULL && cfbuf->nextpos < /* 262114 */ cfbuf->size){
        cfbuf->cflogs[cfbuf->nextpos++] = (CFLog){(uint32_t)(dst), (uint32_t)(src)};
        return;
    }
    flush_cfbuf();
    // printf("cflow_logger: cfbuf: %p, cfbuf.cflogs: %p, cfbuf.size: %zu, cfbuf.nextpos: %zu\n", cfbuf, cfbuf->cflogs, cfbuf->size, cfbuf->nextpos);
    
    // printf("cflow_logger: cflog[%zu].src: %p, .dst: %p\n", cfbuf->nextpos-1, cfbuf->cflogs[cfbuf->nextpos-1].srcaddr, cfbuf->cflogs[cfbuf->nextpos-1].dstaddr);

    // cfbuf->cflogs[cfbuf->nextpos].srcaddr = (uint32_t)src;
    // cfbuf->cflogs[cfbuf->nextpos].dstaddr = (uint32_t)dst;
    // cfbuf->nextpos++;
    // printf("cflow_logger: nextpos = %zu\n", cfbuf->nextpos);

    return;
}

// 3 similar trace functions with different names for instrumentation and optimization
void __attribute__((noinline)) tracecall(void* dest_to){
    void *from = __builtin_return_address(0);
    // printf("tracecall: %p -> %p\n", from, dest_to);
    cflow_logger(dest_to, from);
    return;
}

void __attribute__((noinline)) traceret(void* dest_to){
    void *from = __builtin_return_address(0);
    // printf("traceret: %p -> %p\n", from, dest_to);
    cflow_logger(dest_to, from);
    return;
}

void __attribute__((noinline)) traceindirectbr(void* dest_to){
    void *from = __builtin_return_address(0);
    // printf("traceindirectbr: %p -> %p\n", from, dest_to);
    cflow_logger(dest_to, from);
    return;
}