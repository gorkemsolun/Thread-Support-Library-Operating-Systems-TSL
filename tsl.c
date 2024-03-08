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
int tsl_create_thread (void (*tsf)(void *), void *targ);
int tsl_yield (int tid);
int tsl_exit();
int tsl_join(int tid);
int tsl_cancel(int tid);
int tsl_gettid();


int 
tsl_init ( int salg)
{
    return (0);
    // we put return(0) as a placeholder.
    // read project about what to return.
    //and change return() accordingly. 

    int success = 0;
    if(success) {
        return 0;
    }
    else {
        return TSL_ERROR;
    }

}



int
tsl_create_thread(void (*tsf)(void *), void *targ)
{
    return (0);
}



int
tsl_yield(int tid)
{
    return (0);
}


int 
tsl_exit()
{
    return (0);
}

int
tsl_join(int tid)
{
    return (0);
}


int
tsl_cancel(int tid)
{
    /*
    
        With this function, the calling thread will cancel another thread. 
        The thread to be cancelled (target thread) is indicated with the tid argument. 
        
        The cancellation will be asynchronous.

    */

    return (0);
}


int
tsl_gettid()
{
    /*
    
        Returns the thread id (tid) of the calling thread; that
        means, the thread identifier assigned by the library.
    
    */

    return (0);
}