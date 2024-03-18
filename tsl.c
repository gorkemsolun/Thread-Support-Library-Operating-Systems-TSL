#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <assert.h>
#include <time.h>

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
// these additional functions will not be available for applications directly. 

// TCB structure
typedef struct TCB {
    int tid; // thread identifier
    unsigned int state; // thread state
    ucontext_t context; // pointer to context structure

    /*
        typedef struct ucontext_t
        {
        unsigned long int __ctx(uc_flags);
        struct ucontext_t *uc_link;
        stack_t uc_stack;
        mcontext_t uc_mcontext;
        sigset_t uc_sigmask;
        struct _libc_fpstate __fpregs_mem;
        } ucontext_t;
    */

    char* stack; // pointer to stack
} TCB;

// Queue implementation
typedef struct QueueNode {
    TCB* tcb; // pointer to TCB
    struct QueueNode* next; // pointer to next node
} QueueNode;

typedef struct tcbQueue {
    QueueNode* front; // pointer to front of the queue
    QueueNode* rear; // pointer to rear of the queue
} tcbQueue;

tcbQueue* createTCBQueue() {
    tcbQueue* queue = (tcbQueue*)malloc(sizeof(tcbQueue));
    queue->front = NULL;
    queue->rear = NULL;
    return queue;
}

// Create a new ready queue
void enqueue(tcbQueue* queue, TCB* tcb) {
    QueueNode* newNode = (QueueNode*)malloc(sizeof(QueueNode));
    newNode->tcb = tcb;
    newNode->next = NULL;

    if (queue->rear == NULL) {
        queue->front = newNode;
        queue->rear = newNode;
    } else {
        queue->rear->next = newNode;
        queue->rear = newNode;
    }
}

// Dequeue from the queue
TCB* dequeue(tcbQueue* queue) {
    if (queue->front == NULL) {
        return NULL;
    }

    QueueNode* temp = queue->front;
    TCB* tcb = temp->tcb;
    queue->front = queue->front->next;

    if (queue->front == NULL) {
        queue->rear = NULL;
    }

    free(temp);
    printf("Dequeued tid: %d\n", tcb->tid);
    return tcb;
}

// Check if the queue is empty
int isReadyQueueEmpty(tcbQueue* queue) {
    return queue->front == NULL;
}

// Queue implementation end

// Define states
#define TSL_RUNNING 1
#define TSL_READY 2
#define TSL_EXIT 3

// Global variables
unsigned int readyQueueSize = 0;
unsigned int schedulingAlg; // 1 is FCFS, 2 is random
int currentThreadCount = 0; // Current number of threads
tcbQueue* readyQueue; // ready queue
tcbQueue* runningQueue; // running queue
tcbQueue* exitedQueue; // when a process calls exit then it will be moved to this queue
int next_tid = 1; // next thread id
int tsl_init(int salg);
int tsl_create_thread(void (*tsf)(void*), void* targ);
int tsl_yield(int tid);
int tsl_exit();
int tsl_join(int tid);
int tsl_cancel(int tid);
int tsl_gettid();
TCB* removeFromQueue(tcbQueue* queue, int tid);

// TODO: STUB should be implemented
// TODO: STUB SHOULD BE PUT IN THE CORRECT PLACE
// stub wrapper function for the thread start function
void stub(void (*tsf) (void*), void* targ) {
    // new thread will start its execution here
    tsf(targ); // then we will call the thread start function

    // tsf will retun to here
    tsl_exit(); // now ask for termination
}


// This is the main function of the library and library will be initialized here
// This function will be called exatcly once
// salg is the scheduling algorithm to be used
int tsl_init(int salg) {
    schedulingAlg = salg;
    readyQueue = createTCBQueue();
    runningQueue = createTCBQueue();
    exitedQueue = createTCBQueue();

    // Add current(main) thread's TCB to running queue
    TCB* mainTCB = (TCB*)malloc(sizeof(TCB));
    mainTCB->tid = next_tid++; // Assuming main thread has tid 1
    mainTCB->state = TSL_RUNNING; // Set state to running
    mainTCB->stack = NULL; // No stack for main thread
    enqueue(runningQueue, mainTCB);

    int success = 1;// TODO: Success should be handled
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
    if (currentThreadCount >= TSL_MAXTHREADS) {
        return TSL_ERROR; // Maximum number of threads reached
    }

    // Create a new TCB for the new thread
    TCB* newTCB = (TCB*)malloc(sizeof(TCB));
    newTCB->tid = next_tid++;
    newTCB->state = TSL_READY;
    newTCB->stack = (char*)malloc(TSL_STACKSIZE);

    enqueue(readyQueue, newTCB); // Add the new thread to the ready queue

    currentThreadCount++; // Increment the number of threads

    // Create a new context for the new thread
    getcontext(&newTCB->context);

    // TODO: This part will be refactored later
    /*----------------------------------------------------------------------------------------------------------------*/

    newTCB->context.uc_stack.ss_sp = newTCB->stack; // Allocate stack frame for the tsf 
    newTCB->context.uc_stack.ss_size = TSL_STACKSIZE; // Specify tsf stack frame size
    newTCB->context.uc_link = NULL; // May be unnecessary but this value will always be NULL for this project

    newTCB->context.uc_mcontext.gregs[REG_EIP] = (unsigned int)stub; // Set the instruction pointer to the stub function
    
    size_t total_size = sizeof(tsf) + sizeof(targ);

    newTCB->context.uc_mcontext.gregs[REG_ESP] = (unsigned long)(newTCB->stack + TSL_STACKSIZE - total_size);
    // change the stack pointer field (REG ESP) of the context structure to point to the top of the new stack. 

    void* ptr = (void*) (newTCB->stack + TSL_STACKSIZE - total_size) + sizeof(void*);
    *(void**)ptr = tsf; // TODO: I am not sure for this line, may require testing this part will be one of the following:
                        // *(void**)ptr = tsf;
                        // *(void**)ptr = *tsf;
                        // *(void**)ptr = (*tsf)(void*);

    ptr = ptr + sizeof(void*);
    *(void**)ptr = targ;

    // TODO: Now only thing need to be done is to call "setcontext(&newTCB->context)" which will be done in tsl_yield(int id)

    int success = 1; // TODO: Success should be handled
    if (success) {
        return newTCB->tid;
    } else {
        return TSL_ERROR;
    }

    /*----------------------------------------------------------------------------------------------------------------*/
}

TCB* FCFS() {
    TCB* thread = dequeue(readyQueue);
    if (readyQueueSize > 0) {
       readyQueueSize--; 
    }
    return thread;
}

TCB* Random() {
    srand(time(NULL));
    int randomIndex = rand() % readyQueueSize + 1;
    QueueNode* iterator = readyQueue->front;
    int index = 1;
    int tid = -1;
    while(iterator != NULL) {
        if(index == randomIndex) {
            tid = iterator->tcb->tid;
        }
        iterator = iterator->next;
    }
    TCB* thread = NULL;
    if(tid >0) {
        thread = removeFromQueue(readyQueue, tid); 
    }
   
    if (readyQueueSize > 0) {
       readyQueueSize--; 
    }
    return thread;
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
    printf("Test ediyoz1.............   ");
    int next_tid;
    TCB* calleeThread = NULL;
    TCB* callerThread = runningQueue->front->tcb;
    dequeue(runningQueue); //remove the caller thread
    printf("Test ediyoz2.............   ");
    callerThread->state = TSL_READY;
    enqueue(readyQueue, callerThread); // add the caller thread to ready queue
    readyQueueSize++;
    printf("Test ediyoz3.............   ");
    if (tid == TSL_ANY) { // pick according to the schedule algorithm
        if (schedulingAlg == ALG_FCFS) {
            calleeThread = FCFS();
        } else if (schedulingAlg == ALG_RANDOM) { //pick a random element from the ready queue (this could result in starvation)
            calleeThread =  Random();
        }
    } else { //pick the thread specified by tid
        
        calleeThread = removeFromQueue(readyQueue, tid);
        
        if (calleeThread != NULL) {
            readyQueueSize--;
        } else {
            /*If tid parameter is a positive integer but there is no ready thread
            with that tid, the function will return immediately without yielding to any
            thread.*/
            
            //yield the cpu back to the caller.
            removeFromQueue(readyQueue, callerThread->tid);
            readyQueueSize--;
            callerThread->state = TSL_RUNNING;
            enqueue(runningQueue, callerThread);
            return -1;
        }
    }


    //now that the thread to be run is set, put it into the running queue and run it
    next_tid = calleeThread->tid;

    getcontext(&(callerThread->context)); //save the callers context

    if (callerThread->state == TSL_READY) {
        //it means the first return, yield to the callee
        calleeThread->state = TSL_RUNNING;
        enqueue(runningQueue, calleeThread); //insert the callee thread
        setcontext(&(calleeThread->context)); //switch context
    } else {
        // it means the second return, some other thread has yielded to this thread
        
        printf("thread %d yielded to thread %d    ",callerThread->tid, next_tid );
        return next_tid;
    }
}

// terminates the calling thread
// TCB and stack of the calling thread will be kept until another thread calls tsl_join with the tid of the calling thread
// this function will not return
int tsl_exit() {
    //move from running to exited queue
    TCB* currentThread = dequeue(runningQueue);
  
    currentThread->state = TSL_EXIT; 
 
    enqueue(exitedQueue, currentThread);

    //This enclosed section could be replaced by yield function
    // I wrote this because I thought that we do not need to save the context of the thread that is exiting
    // I might be wrong...
    //---------------------------------------------------------------------------
    TCB* calleeThread = NULL;
    if (schedulingAlg == ALG_FCFS) {
        calleeThread = FCFS();
    } else if (schedulingAlg == ALG_RANDOM) { //pick a random element from the ready queue (this could result in starvation)
        calleeThread = Random();
    }
    if (calleeThread == NULL) {
        // readyQueue is empty, free all that is left in the exited queue (if there are any left due to not being joined)
        TCB* temp = dequeue(exitedQueue);
        while(temp != NULL) {
            free(temp->stack);
            free(temp);
            temp = dequeue(exitedQueue);
        }
    } else {
        calleeThread->state = TSL_RUNNING;
        enqueue(runningQueue, calleeThread);
        setcontext(&(calleeThread->context));
    }
    //---------------------------------------------------------------------------
    return (0);
}

// waits for the termination of another thread with the given tid, then returns
// before returning, the TCB and stack of the terminated thread will be deallocated
int tsl_join(int tid) {
    int result = tsl_yield(tid);

    if (result == -1) {
        //no such thread in the ready queue
        return -1;
    }

    while(1) {
        QueueNode* iterator = exitedQueue->front;
        TCB* joinedThread = NULL;
        while(iterator != NULL) {
            if(iterator->tcb->tid == tid) {
                joinedThread = iterator->tcb;
                break;
            }
            iterator = iterator->next;
        }

        if(joinedThread != NULL) {
            removeFromQueue(exitedQueue, joinedThread->tid);
            free(joinedThread->stack);
            free(joinedThread);
            break;
        } else {
            tsl_yield(TSL_ANY);
        } 
    } 
    
    
    return tid;
}

// cancels another thread with the given tid asynchronously, the target thread will be immediately terminated
int tsl_cancel(int tid) {

    TCB* tcb = removeFromQueue(readyQueue, tid); //if does not exist in ready queue, it will return NULL
    if(tcb == NULL){
        tcb = removeFromQueue(runningQueue, tid); 
        if (tcb == NULL) {
            return TSL_ERROR; 
        }
    }
    
    free(tcb->stack);
    free(tcb);

    return TSL_SUCCESS;
}

// helper function to remove a thread from the queue
// finds the thread id of the calling thread and removes it
// returns the TCB pointer of removed thread, returns NULL if tcbQueue is empty

TCB* removeFromQueue(tcbQueue* queue, int tid){
    if(queue == NULL || queue->front == NULL){
        return NULL;
    }

    QueueNode* iterator = queue->front;
    QueueNode* prev = NULL;

    while(iterator != NULL){
        if(iterator->tcb->tid == tid){
            if(prev == NULL){
                queue->front = iterator->next;
            } else {
                prev->next = iterator->next;
            }

            if(iterator->next == NULL){
                queue->rear = prev;
            }

            TCB* tcb = iterator->tcb;
            free(iterator);
            return tcb;
        }

        prev = iterator;
        iterator = iterator->next;
    }

    return NULL;

}

// returns the thread id of the calling thread
// returns 0 if runningQueue is empty
int tsl_gettid() {
    if(runningQueue != NULL && runningQueue->front != NULL) {
        return runningQueue->front->tcb->tid;
    }

    return 0;
}