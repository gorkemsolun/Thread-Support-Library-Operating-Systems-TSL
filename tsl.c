#include <pthread.h>
#include "tsl.h"
#include "common.h"

int tsl_init(int salg) {
    
    /*
        An application will call this function exactly once before creating any threads. 

        The parameter salg is used to indicate the scheduling algorithm the library will use.
    */

    int success = 0;
    if(success) {
        return 0;
    }
    else {
        return TSL_ERROR;
    }

}

int tsl_create_thread(void (*tsf)(void *), void *targ) {

}

int tsl_yield(int tid) {

}

int tsl_exit() {

}

int tsl_join(int tid) {

}

int tsl_cancel(int tid) {

    /*
    
        With this function, the calling thread will cancel another thread. 
        The thread to be cancelled (target thread) is indicated with the tid argument. 
        
        The cancellation will be asynchronous.

    */

}

int tsl_gettid() {
    
    /*
    
        Returns the thread id (tid) of the calling thread; that
        means, the thread identifier assigned by the library.
    
    */

    return 0;

}
