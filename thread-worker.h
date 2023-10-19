// File:	worker_t.h

// List all group member's name:
// username of iLab:
// iLab Server:

// Line 59 
#ifndef WORKER_T_H
#define WORKER_T_H

#define _GNU_SOURCE

/* To use Linux pthread Library in Benchmark, you have to comment the USE_WORKERS macro */
#define USE_WORKERS 1
#define STACK_SIZE SIGSTKSZ
#define READY 0
#define RUNNING 1
#define BLOCKED 2
#define Quantum 10

/* Include enums or other helpful types */
enum boolean {false, true};

/* include lib header files that you need here: */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>

typedef uint worker_t;

typedef struct TCB {
	/* add important states in a thread control block */
	// thread Id
	worker_t id;
	// thread status
	int status;
	// thread context
	ucontext_t context;
	// thread stack
	void *stack;
	// thread priority
	// And more ...
	int QuantumCounter; //This variable is meant to be incremented everytime the thread has run for quantum.
	int TurnAroundCounter; //This variable is incremented every single quantum that happens in total since
						  //it is first added to the Quene, until the thread is finished. 
	int ResponseTimeCounter; //This variable is incremented for every quantum the thread has to wait after
							//it is added into the Quene and has NOT been scheduled yet. 

	// Possible add metrics for completion time, arrival time, 
	// first run time, number of context switches for per thread TCB
	// 

	// YOUR CODE HERE
} tcb; 





/* define your data structures here: */
// Feel free to add your own auxiliary data structures (linked list or queue etc...)

// YOUR CODE HERE
static void sched_mlfq();
static void sched_psjf();
static void schedule();

struct Node {
    tcb *data; // Pointer to the struct
    struct Node *next; // Pointer to the next node
};

/* mutex struct definition */
typedef struct worker_mutex_t {
	/* add something here */
	tcb *thread;
	struct Node *mutexQueneHead; 
	enum boolean lock;
	// YOUR CODE HERE
}worker_mutex_t;


/*typedef struct mutexNode {
    mutex *data; // Pointer to the struct
    struct mutexNode *next; // Pointer to the next node
}mutexNode;*/

void enqueue(tcb *thread); //insert tcb at end of runqueue
tcb* dequeue(tcb *thread); //delete specific tcb

void handler(int signum);


/* Function Declarations: */

/* create a new thread */
int worker_create(worker_t * thread, pthread_attr_t * attr, void
    *(*function)(void*), void * arg);

/* give CPU pocession to other user level worker threads voluntarily */
int worker_yield();

/* terminate a thread */
void worker_exit(void *value_ptr);

/* wait for thread termination */
int worker_join(worker_t thread, void **value_ptr);

/* initial the mutex lock */
int worker_mutex_init(worker_mutex_t *mutex, const pthread_mutexattr_t
    *mutexattr);

/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex);

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex);

/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex);


/* Function to print global statistics. Do not modify this function.*/
void print_app_stats(void);

#ifdef USE_WORKERS
#define pthread_t worker_t
#define pthread_mutex_t worker_mutex_t
#define pthread_create worker_create
#define pthread_exit worker_exit
#define pthread_join worker_join
#define pthread_mutex_init worker_mutex_init
#define pthread_mutex_lock worker_mutex_lock
#define pthread_mutex_unlock worker_mutex_unlock
#define pthread_mutex_destroy worker_mutex_destroy
#endif

#endif
