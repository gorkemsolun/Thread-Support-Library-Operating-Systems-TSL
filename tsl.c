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
    char* stack; // pointer to stack

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
} TCB;

// Queue implementation
typedef struct QueueNode {
    TCB* tcb; // pointer to TCB
    struct QueueNode* next; // pointer to next node
} QueueNode;

typedef struct TCBQueue {
    QueueNode* front; // pointer to front of the queue
    QueueNode* rear; // pointer to rear of the queue
    int size; // size of the queue
} TCBQueue;

TCBQueue* createTCBQueue() {
    TCBQueue* queue = (TCBQueue*)malloc(sizeof(TCBQueue));
    queue->front = NULL;
    queue->rear = NULL;
    queue->size = 0;
    return queue;
}

// Create a new ready queue
int enqueue(TCBQueue* queue, TCB* tcb) {
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

    if (queue->front == NULL || queue->rear == NULL) {
        return TSL_ERROR;
    } else {
        queue->size++;
        return TSL_SUCCESS;
    }
}

// Dequeue from the queue
TCB* dequeue(TCBQueue* queue) {
    if (queue->front == NULL) {
        return NULL;
    }

    QueueNode* temp = queue->front;
    TCB* tcb = temp->tcb;
    queue->front = queue->front->next;

    if (queue->front == NULL) {
        queue->rear = NULL;
    }

    queue->size--;
    free(temp);
    return tcb;
}

// helper function to remove a thread from the queue
// finds the thread id of the calling thread and removes it
// returns the TCB pointer of removed thread, returns NULL if TCBQueue is empty
TCB* removeFromQueue(TCBQueue* queue, int tid) {
    if (queue == NULL || queue->front == NULL) {
        return NULL;
    }

    QueueNode* iterator = queue->front;
    QueueNode* prev = NULL;

    while (iterator != NULL) {
        if (iterator->tcb->tid == tid) {
            if (prev == NULL) {
                queue->front = iterator->next;
            } else {
                prev->next = iterator->next;
            }

            if (iterator->next == NULL) {
                queue->rear = prev;
            }

            TCB* tcb = iterator->tcb;
            free(iterator);
            queue->size--;
            return tcb;
        }

        prev = iterator;
        iterator = iterator->next;
    }

    return NULL;
}

// Queue implementation end

// Define states
#define TSL_RUNNING 1
#define TSL_READY 2
#define TSL_EXIT 3

// Global variables
unsigned int schedulingAlg; // 1 is FCFS, 2 is random
int currentThreadCount = 0; // Current number of threads
TCBQueue* readyQueue; // ready queue
TCBQueue* runningQueue; // running queue
TCBQueue* exitedQueue; // when a process calls exit then it will be moved to this queue
int next_tid = 1; // next thread id
int tsl_init(int salg);
int tsl_create_thread(void (*tsf)(void*), void* targ);
int tsl_yield(int tid);
int tsl_exit();
int tsl_join(int tid);
int tsl_cancel(int tid);
int tsl_gettid();


// Helper functions

// Scheduling algorithms
TCB* FCFS() {
    TCB* thread = dequeue(readyQueue);

    return thread;
}

TCB* Random() {
    if (readyQueue->size == 0) {
        return NULL;
    }

    srand(time(NULL) ^ getpid()); // TODO: is this really produces a random number?
    int randomIndex = rand() % readyQueue->size;
    QueueNode* iterator = readyQueue->front;

    for (int i = 0; i < randomIndex; i++) {
        iterator = iterator->next;
    }

    TCB* thread = iterator->tcb;
    removeFromQueue(readyQueue, thread->tid);

    return thread;
}


// Library functions

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

    if (readyQueue == NULL || runningQueue == NULL || exitedQueue == NULL) {
        return TSL_ERROR;
    }

    // Add current(main) thread's TCB to running queue
    TCB* mainTCB = (TCB*)malloc(sizeof(TCB));

    if (mainTCB == NULL) {
        return TSL_ERROR;
    }

    mainTCB->tid = next_tid++; // Assuming main thread has tid 1
    mainTCB->state = TSL_RUNNING; // Set state to running
    mainTCB->stack = NULL; // No stack for main thread

    // Add main thread to running queue
    if (enqueue(runningQueue, mainTCB) == TSL_ERROR) {
        return TSL_ERROR;
    } else {
        return TSL_SUCCESS;
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

    // Add the new thread to the ready queue
    if (enqueue(readyQueue, newTCB) == TSL_ERROR) {
        return TSL_ERROR;
    }

    currentThreadCount++; // Increment the number of threads

    getcontext(&newTCB->context);// Create a new context for the new thread

    // TODO: This part will be refactored later (gorkem done this part but it should be checked again)

    newTCB->context.uc_mcontext.gregs[REG_EIP] = (unsigned int)stub; // Set the instruction pointer to the stub function

    newTCB->context.uc_stack.ss_sp = newTCB->stack; // Allocate stack frame for the tsf 
    newTCB->context.uc_stack.ss_size = TSL_STACKSIZE; // Specify tsf stack frame size
    newTCB->context.uc_link = NULL; // May be unnecessary but this value will always be NULL for this project

    size_t total_size = sizeof(tsf) + sizeof(targ);
    // change the stack pointer field (REG ESP) of the context structure to point to the top of the new stack.
    newTCB->context.uc_mcontext.gregs[REG_ESP] = (unsigned long)(newTCB->stack + TSL_STACKSIZE - total_size);

    void* ptr = (void*)(newTCB->stack + TSL_STACKSIZE - total_size) + sizeof(void*);
    *(void**)ptr = tsf;
    // TODO: I am not sure for this line, may require testing this part will be one of the following:
    // Gorkem: Functions can be passed, so it should be correct to pass the function pointer directly
    // *(void**)ptr = tsf;
    // *(void**)ptr = *tsf;
    // *(void**)ptr = (*tsf)(void*);

    ptr = ptr + sizeof(void*);
    *(void**)ptr = targ;

    return newTCB->tid;
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
    int next_tid;
    // Callee and caller threads ***DO NOT confuse with the caller and callee functions***
    TCB* calleeThread = NULL;
    TCB* callerThread = runningQueue->front->tcb;

    // remove the caller thread
    if (dequeue(runningQueue) == NULL) {
        return TSL_ERROR;
    }

    callerThread->state = TSL_READY;

    // add the caller thread to ready queue
    if (enqueue(readyQueue, callerThread) == TSL_ERROR) {
        return TSL_ERROR;
    }

    // pick according to the schedule algorithm
    if (tid == TSL_ANY) {
        if (schedulingAlg == ALG_FCFS) {
            calleeThread = FCFS();
        } else if (schedulingAlg == ALG_RANDOM) { //pick a random element from the ready queue (this could result in starvation)
            calleeThread = Random();
        }

        // TODO: Should this be checked?
        if (calleeThread == NULL) {
            return TSL_ERROR;
        }
    } else { //pick the thread specified by tid
        calleeThread = removeFromQueue(readyQueue, tid);

        /*If tid parameter is a positive integer but there is no ready thread
        with that tid, the function will return immediately without yielding to any
        thread.*/
        if (calleeThread == NULL) {// yield the cpu back to the caller.
            removeFromQueue(readyQueue, callerThread->tid);
            callerThread->state = TSL_RUNNING;
            if (enqueue(runningQueue, callerThread) == TSL_ERROR) {
                printf("Error in enqueueing the caller thread to running queue\n");
                return TSL_ERROR;
            }
            return TSL_ERROR;
        }
    }

    //now that the thread to be run is set, put it into the running queue and run it
    next_tid = calleeThread->tid;

    getcontext(&(callerThread->context)); //save the callers context

    if (callerThread->state == TSL_READY) {
        //it means the first return, yield to the callee
        calleeThread->state = TSL_RUNNING;
        // insert the callee thread
        if (enqueue(runningQueue, calleeThread) == TSL_ERROR) {
            return TSL_ERROR;
        }
        setcontext(&(calleeThread->context)); //switch context
    } else {
        // it means the second return, some other thread has yielded to this thread
        return next_tid;
    }

    return TSL_ERROR; // EDIZ TODO: what should be returned here?
}

// terminates the calling thread
// TCB and stack of the calling thread will be kept until another thread calls tsl_join with the tid of the calling thread
// this function will not return
int tsl_exit() {
    //move from running to exited queue
    TCB* currentThread = dequeue(runningQueue);

    if (currentThread == NULL) {
        return TSL_ERROR;
    }

    currentThread->state = TSL_EXIT;

    if (enqueue(exitedQueue, currentThread) == TSL_ERROR) {
        return TSL_ERROR;
    }

    TCB* calleeThread = NULL;
    if (schedulingAlg == ALG_FCFS) {
        calleeThread = FCFS();
    } else if (schedulingAlg == ALG_RANDOM) { //pick a random element from the ready queue (this could result in starvation)
        calleeThread = Random();
    }

    if (calleeThread == NULL) {
        // readyQueue is empty, free all that is left in the exited queue (if there are any left due to not being joined)
        TCB* temp = dequeue(exitedQueue);
        while (temp != NULL) {
            free(temp->stack);
            free(temp);
            temp = dequeue(exitedQueue);
        }
    } else {
        calleeThread->state = TSL_RUNNING;
        if (enqueue(runningQueue, calleeThread) == TSL_ERROR) {
            return TSL_ERROR;
        }
        setcontext(&(calleeThread->context));
    }
    return TSL_SUCCESS;
}

// waits for the termination of another thread with the given tid, then returns
// before returning, the TCB and stack of the terminated thread will be deallocated
int tsl_join(int tid) {
    //no such thread in the ready queue
    if (tsl_yield(tid) == TSL_ERROR) {
        return  TSL_ERROR;
    }

    while (1) {
        QueueNode* iterator = exitedQueue->front;
        TCB* joinedThread = NULL;
        while (iterator != NULL) {
            if (iterator->tcb->tid == tid) {
                joinedThread = iterator->tcb;
                break;
            }
            iterator = iterator->next;
        }

        if (joinedThread != NULL) {
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
    TCB* toBeCancelled = removeFromQueue(readyQueue, tid); //if does not exist in ready queue, it will return NULL
    if (toBeCancelled == NULL) {
        toBeCancelled = removeFromQueue(runningQueue, tid);
        if (toBeCancelled == NULL) {
            return TSL_ERROR;
        }
    }

    if (getpid() == toBeCancelled->tid){
        printf("Cannot cancel the caller thread, use exit()\n");
        return TSL_ERROR;
    }
    
    toBeCancelled->state = TSL_EXIT;

    if (enqueue(exitedQueue, toBeCancelled) == TSL_ERROR) {
        return TSL_ERROR;
    }
    
    return TSL_SUCCESS;
}

// returns the thread id of the calling thread
// returns 0 if runningQueue is empty
int tsl_gettid() {
    if (runningQueue != NULL && runningQueue->front != NULL) {
        return runningQueue->front->tcb->tid;
    }

    return 0; // TODO: Is this correct?
}