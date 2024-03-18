#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <assert.h>

/* We want the extra information from these definitions */
#ifndef __USE_GNU
#define __USE_GNU
#endif /* __USE_GNU */

#include <ucontext.h>
#include "tsl.h"


void* foo(void* v);

void stub(void (*tsf)(void*), void* targ);

// TCB structure
typedef struct TCB {
    int tid;               // thread identifier
    unsigned int state;    // thread state
    ucontext_t context;    // context structure
    char* stack;           // pointer to stack
} TCB;

int main() {
    TCB* newTCB = (TCB*)malloc(sizeof(TCB));
    newTCB->tid = 1;
    newTCB->state = 2;

    newTCB->stack = (char*)malloc(TSL_STACKSIZE); // Allocate memory for the stack

    // Create a new context for the new thread
    getcontext(&newTCB->context);

    void (*tsf)(void*) = (void*)&foo;
    void* targ = 4;

    newTCB->context.uc_stack.ss_sp = newTCB->stack;
    newTCB->context.uc_stack.ss_size = TSL_STACKSIZE;
    newTCB->context.uc_link = NULL;
    newTCB->context.uc_mcontext.gregs[REG_EIP] = (unsigned long)stub;

    size_t size_tsf = sizeof(tsf);  // Calculate size of function pointer
    size_t size_targ = sizeof(targ);  // Calculate size of void pointer

    size_t total_size = size_tsf + size_targ;  // Calculate total size

    //newTCB->context.uc_mcontext.gregs[REG_ESP] = (unsigned long)(newTCB->stack + TSL_STACKSIZE - sizeof(void*));
    newTCB->context.uc_mcontext.gregs[REG_ESP] = (unsigned long)(newTCB->stack + TSL_STACKSIZE - total_size);

    //void* ptr = (void*) newTCB->stack + TSL_STACKSIZE;
    printf("%p\n", (void*) (newTCB->stack + TSL_STACKSIZE - total_size));
    void* ptr = (void*) (newTCB->stack + TSL_STACKSIZE - total_size) + sizeof(void*);
    printf("%p\n", (void*) ptr);
    *(void**)ptr = (void*)&foo;
    ptr = ptr + sizeof(void*);
    *(void**)ptr = targ;
    /*
    char* new_stack = (char*)malloc(sizeof(void) + sizeof(void*));
    printf("%d\n",sizeof(foo));
    printf("%d\n",sizeof(void*));
    */

    //makecontext(&newTCB->context, (void (*)(void))tsf, 1, targ);

    // Start execution of the new context
    setcontext(&newTCB->context);

    return 0;
}

void* foo(void* v) {
    int count = 1;
    int mytid = 31;

    printf("Thread %d started running (first time); at the start function\n", v);
    return NULL;
}

void stub(void (*tsf)(void*), void* targ) {
    printf("tsf: %p\n", (void*)&tsf);
    printf("targ: %p\n", (void*)&targ);

    printf("Hello World!\n");
    tsf(targ);
    exit(0);
}