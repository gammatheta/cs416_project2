// File:	thread-worker.c

// List all group member's name:
// username of iLab:
// iLab Server:

#include "thread-worker.h"

//Global counter for total context switches and 
//average turn around and response time
long tot_cntx_switches=0;
double avg_turn_time=0;
double avg_resp_time=0;


// INITAILIZE ALL YOUR OTHER VARIABLES HERE
// YOUR CODE HERE
#define STACK_SIZE SIGSTKSZ
uint idcounter = 0;
ucontext_t mainctx,schedulerctx;
void *schedstack;
int ctxswitch = 0;
enum Boolean fstrun = true;


/* create a new thread */
int worker_create(worker_t * thread, pthread_attr_t * attr, 
                      void *(*function)(void*), void * arg) {

       // - create Thread Control Block (TCB)
       // - create and initialize the context of this worker thread
       // - allocate space of stack for this thread to run
       // after everything is set, push this thread into run queue and 
       // - make it ready for the execution.

       // YOUR CODE HERE
	   if(fstrun){//if the first run of worker_create function
		tcb *thread = malloc(sizeof(tcb));

		thread->id = idcounter;
		idcounter++;
		thread->status = READY;
		
		if (getcontext(&(thread->context)) < 0){
			perror("getcontext");
			exit(1);
		}

		if(getcontext(&schedulerctx) < 0){
			perror("getcontext");
			exit(1);
		}

		if (getcontext(&mainctx) < 0){
			perror("getcontext");
			exit(1);
		}

		schedstack = malloc(STACK_SIZE);

		/* Setup context that we are going to use */
		schedulerctx.uc_link=NULL;
		schedulerctx.uc_stack.ss_sp= schedstack;
		schedulerctx.uc_stack.ss_size=STACK_SIZE;
		schedulerctx.uc_stack.ss_flags=0;

		makecontext(&schedulerctx, (void*)&schedule,0);

		//Set up scheduler
		swapcontext(&mainctx, &schedulerctx); //after swapcontext returns from scheduler, will go to next line

		/* Setup context that we are going to use */
		thread->stack = malloc(STACK_SIZE);

		(thread->context).uc_link= &schedulerctx; //when thread finishes, will return to scheduler context
		(thread->context).uc_stack.ss_sp= thread->stack;
		(thread->context).uc_stack.ss_size=STACK_SIZE;
		(thread->context).uc_stack.ss_flags=0;

		makecontext(&(thread->context), (void*)function, 1, arg);

		//add tcb to runqueue
		//switch to scheduler context and run thread based on scheduling protocol
		//continue building runqueue or regular execution 

		fstrun = false;

	   }else{//if not the first run of worker_create function
		tcb *thread = malloc(sizeof(tcb));

		thread->id = idcounter;
		idcounter++;
		thread->status = READY;
		
		if (getcontext(&(thread->context)) < 0){
			perror("getcontext");
			exit(1);
		}
		
		/* Setup context that we are going to use */
		thread->stack = malloc(STACK_SIZE);

		(thread->context).uc_link= &schedulerctx; //when thread finishes, will return to scheduler context
		(thread->context).uc_stack.ss_sp= thread->stack;
		(thread->context).uc_stack.ss_size=STACK_SIZE;
		(thread->context).uc_stack.ss_flags=0;

		makecontext(&(thread->context), (void*)function, 1, arg);

		//add tcb to runqueue
		//switch to scheduler context and run thread based on scheduling protocol
	   }
	
    return 0;
};

/* give CPU possession to other user-level worker threads voluntarily */
int worker_yield() {
	
	// - change worker thread's state from Running to Ready
	// - save context of this thread to its thread control block
	// - switch from thread context to scheduler context

	// YOUR CODE HERE
	
	return 0;
};

/* terminate a thread */
void worker_exit(void *value_ptr) {
	// - de-allocate any dynamic memory created when starting this thread

	// YOUR CODE HERE
};


/* Wait for thread termination */
int worker_join(worker_t thread, void **value_ptr) {
	
	// - wait for a specific thread to terminate
	// - de-allocate any dynamic memory created by the joining thread
  
	// YOUR CODE HERE
	return 0;
};

/* initialize the mutex lock */
int worker_mutex_init(worker_mutex_t *mutex, 
                          const pthread_mutexattr_t *mutexattr) {
	//- initialize data structures for this mutex

	// YOUR CODE HERE
	return 0;
};

/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex) {

        // - use the built-in test-and-set atomic function to test the mutex
        // - if the mutex is acquired successfully, enter the critical section
        // - if acquiring mutex fails, push current thread into block list and
        // context switch to the scheduler thread

        // YOUR CODE HERE
        return 0;
};

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex) {
	// - release mutex and make it available again. 
	// - put threads in block list to run queue 
	// so that they could compete for mutex later.

	// YOUR CODE HERE
	return 0;
};


/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex) {
	// - de-allocate dynamic memory created in worker_mutex_init

	return 0;
};

/* scheduler */
static void schedule() {
	// - every time a timer interrupt occurs, your worker thread library 
	// should be contexted switched from a thread context to this 
	// schedule() function

	// - invoke scheduling algorithms according to the policy (PSJF or MLFQ)

	// if (sched == PSJF)
	//		sched_psjf();
	// else if (sched == MLFQ)
	// 		sched_mlfq();

	// YOUR CODE HERE

// - schedule policy
#ifndef MLFQ
	// Choose PSJF
#else 
	// Choose MLFQ
#endif

}

/* Pre-emptive Shortest Job First (POLICY_PSJF) scheduling algorithm */
static void sched_psjf() {
	// - your own implementation of PSJF
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}


/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
	// - your own implementation of MLFQ
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}

//DO NOT MODIFY THIS FUNCTION
/* Function to print global statistics. Do not modify this function.*/
void print_app_stats(void) {

       fprintf(stderr, "Total context switches %ld \n", tot_cntx_switches);
       fprintf(stderr, "Average turnaround time %lf \n", avg_turn_time);
       fprintf(stderr, "Average response time  %lf \n", avg_resp_time);
}


// Feel free to add any other functions you need

// YOUR CODE HERE

struct Queue* createQueue() {
    struct Queue* queue = (struct Queue*)malloc(sizeof(struct Queue));
    queue->head = queue->tail = NULL;
    return queue;
}

void enqueue(struct Queue* queue, struct data) {
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    newNode->data = data;
    newNode->next = NULL;

    if (queue->rear == NULL) {
        queue->head = queue->tail = newNode;
        return;
    }

    queue->rear->next = newNode;
    queue->rear = newNode;
}

void dequeue(struct Queue* queue, Node* node) {
	node* ptr = quene->head;
	node* ptr2 = quene->head;

	if(node == quene->head)
	{
		quene->head = quene->head->next;
		free(ptr);
		return;
	}

	ptr2 = ptr2->next;
	while(ptr2!=NULL)
	{
		if(ptr2 == node)
		{
			if(ptr2->next!=NULL)
			{
				ptr = ptr2->next;
			}
			else
			{
				quene->tail = ptr;
			}
			free(ptr2);
		}
		ptr = ptr->next;
		ptr2 = pt2->next;
	}


}

void enqueue(tcb *thread){//insert tcb at end of runqueue
//make new node and then add thread to node->data

struct Node *newNode = malloc(sizeof(struct Node));

newNode->data = thread;
newNode->next = NULL;

struct Node *ptr = runqueuehead;

	while(ptr->next != NULL){
		ptr = ptr->next;
	}

	ptr->next = newNode;

}
