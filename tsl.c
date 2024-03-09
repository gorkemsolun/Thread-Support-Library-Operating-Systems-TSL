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

// you will implement your library in this file. 
// you can define your internal structures, macros here. 
// such as: #define ...
// if you wish you can use another header file (not necessary). 
// but you should not change the tsl.h header file. 
// you can also define and use global variables here.
// you can define and use additional functions (as many as you wish) 
// in this file; besides the tsl library functions desribed in the assignment. 
// these additional functions will not be available for 
// applications directly. 

int tsl_init(int salg);
int tsl_create_thread(void (*tsf)(void*), void* targ);
int tsl_yield(int tid);
int tsl_exit();
int tsl_join(int tid);
int tsl_cancel(int tid);
int tsl_gettid();

// This is the main function of the library and library will be initialized here
// This function will be called exatcly once
// salg is the scheduling algorithm to be used
int tsl_init(int salg) {

    int success = 1;
    if (success) {
        return 0;
    } else {
        return TSL_ERROR;
    }

}

// creates a new thread
// tsf is the function(the thread start function or root function) to be executed by the new thread
// The application will define this thread start function, and its address will be the first argument
// Function can take one argument of type void*
// targ is a pointer to a value or structure that will be passed to the thread start function, if nothing is to be passed, it will be NULL
// The function will return the thread id(tid) of the new thread, unique among all threads in the application
int tsl_create_thread(void (*tsf)(void*), void* targ) {
    int tid = 0; // TODO: generate a unique thread id

    int success = 1;
    if (success) {
        return tid;
    } else {
        return TSL_ERROR;
    }
}

// yields the processor to another thread, a context switch will occur
// tid is the thread id of the thread to which the processor will be yielded
// if tid is TSL_ANY, the processor will be yielded to any thread selected by the scheduling algorithm from the threads that are ready to run
// selected threads context will be loaded and it will start to execute
// if there is no ready thread to run other than calling thread, scheduler will select the calling thread to run next
// save calling threads context then restore it later
// if tid > 0, but no ready thread with the given tid, the function will return TSL_ERROR
// this function will not return until the calling thread is scheduled to run again
int tsl_yield(int tid) {
    int next_tid = 0; // TODO: select the next thread to run


    // TODO: not return until the calling thread is scheduled to run again
    int success = 1;
    if (success) {
        return next_tid;
    } else {
        return TSL_ERROR; // TODO: no ready thread with the given tid
    }
}

// terminates the calling thread
// TCB and stack of the calling thread will be kept until another thread calls tsl_join with the tid of the calling thread
// this function will not return
int tsl_exit() {
    return (0);
}

// waits for the termination of another thread with the given tid, then returns
// before returning, the TCB and stack of the terminated thread will be deallocated
int tsl_join(int tid) {
    int success = 1;
    if (success) {
        return tid;
    } else {
        return TSL_ERROR; // TODO: no thread with the given tid
    }
}

// cancels another thread with the given tid asynchronously, the target thread will be immediately terminated
int tsl_cancel(int tid) {
    int success = 1;
    if (success) {
        return tid;
    } else {
        return TSL_ERROR; // TODO: no thread with the given tid
    }
}

// returns the thread id of the calling thread
int tsl_gettid() {

    return 0; // TODO: return the thread id of the calling thread
}