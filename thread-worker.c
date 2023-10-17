// File:	thread-worker.c

// List all group member's name:
// username of iLab:
// iLab Server:

//Line 377
#include "thread-worker.h"
#include <string.h>
#include <sys/time.h>

//Global counter for total context switches and 
//average turn around and response time
long tot_cntx_switches=0;
double avg_turn_time=0;
double avg_resp_time=0;


// INITAILIZE ALL YOUR OTHER VARIABLES HERE
// YOUR CODE HERE
uint idcounter = 0;
ucontext_t mainctx,schedulerctx;
void *schedstack;
int ctxswitch = 0;
enum boolean fstrun = true;
struct Node *runqueuehead;
tcb *currThread;


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
		tcb *newthread = malloc(sizeof(tcb));

		newthread->id = idcounter;

		thread = &(newthread->id); //unsure if works

		idcounter++;

		newthread->status = READY;
		
		if (getcontext(&(newthread->context)) < 0){
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
		//swapcontext(&mainctx, &schedulerctx); //after swapcontext returns from scheduler, will go to next line
		schedule();
		/* Setup context that we are going to use */
		newthread->stack = malloc(STACK_SIZE);

		(newthread->context).uc_link= &schedulerctx; //when thread finishes, will return to scheduler context
		(newthread->context).uc_stack.ss_sp= newthread->stack;
		(newthread->context).uc_stack.ss_size=STACK_SIZE;
		(newthread->context).uc_stack.ss_flags=0;

		makecontext(&(newthread->context), (void*)function, 1, arg);

		//add tcb to runqueue
		enqueue(newthread);

		fstrun = false;
		//switch to scheduler context and run thread based on scheduling protocol
		//continue building runqueue or regular execution

		swapcontext(&mainctx, &schedulerctx);


	   }
	   else{//if not the first run of worker_create function
		tcb *newthread = malloc(sizeof(tcb));

		newthread->id = idcounter;

		thread = &(newthread->id); //unsure if works

		idcounter++;
		newthread->status = READY;
		
		if (getcontext(&(newthread->context)) < 0){
			perror("getcontext");
			exit(1);
		}
		
		/* Setup context that we are going to use */
		newthread->stack = malloc(STACK_SIZE);

		(newthread->context).uc_link= &schedulerctx; //when thread finishes, will return to scheduler context
		(newthread->context).uc_stack.ss_sp= newthread->stack;
		(newthread->context).uc_stack.ss_size=STACK_SIZE;
		(newthread->context).uc_stack.ss_flags=0;

		makecontext(&(newthread->context), (void*)function, 1, arg);

		//add tcb to runqueue
		enqueue(newthread);

		//switch to scheduler context and run thread based on scheduling protocol
		//continue building runqueue or regular execution

		swapcontext(&mainctx, &schedulerctx);
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
#define PSJF 1
#else 
	// Choose MLFQ
#define MLFQ 1
#endif

	if(fstrun){//first run of scheduler
		/* Set up runqueue data structure (queue) 
		*  if MLFQ need multiple; if PSJF need only one 
		*  successive calls will require to make nodes in worker_create and add tcb to runqueue
		*/
		if(PSJF){
			
			//check if need anything else for this (ie call sched_psjf())

		}else{//set up MLFQ

		}

		// // Use sigaction to register signal handler
		struct sigaction sa;
		memset(&sa, 0, sizeof (sa));
		sa.sa_handler = &handler;
		sigaction(SIGPROF, &sa, NULL);

		// // Create timer struct
		struct itimerval timer;

		// // Set up what the timer should reset to after the timer goes off
		timer.it_interval.tv_usec = 0; 
		timer.it_interval.tv_sec = 1;

		timer.it_value.tv_usec = 0;
		timer.it_value.tv_sec = 1;

		// // Set the timer up (start the timer)
		setitimer(ITIMER_PROF, &timer, NULL);

		//swapcontext(&schedulerctx, &mainctx); //return back to worker_create func
		return;
	}
	while(runqueuehead!= NULL)
	{
		if(PSJF){
			sched_psjf();
			swapcontext(&schedulerctx, &currThread->context);
			if(currThread!=NULL)
			{
				enqueue(currThread);
			}
			swapcontext(&schedulerctx, &mainctx);
		}
		else{
			sched_mlfq();
		}
	}

	fstrun = true; 

}

/* Pre-emptive Shortest Job First (POLICY_PSJF) scheduling algorithm */
static void sched_psjf() {
	struct Node* ptr = runqueuehead;
	struct Node* ptr2 = runqueuehead;
	struct Node* ptr3 = runqueuehead;
	while(ptr!=NULL)
	{
		
		ptr->data->TurnAroundCounter = ptr->data->TurnAroundCounter+1;
		ptr->data->ResponseTimeCounter = ptr->data->ResponseTimeCounter+1;
		ptr = ptr->next;
	}
	//ptr = runquenehead; 

	int minQc = -1;
	if(ptr2->data->status != 2)
	{
		 minQc = ptr2->data->QuantumCounter;
	}

	ptr2 = ptr2->next;
	while(ptr2!=NULL)
	{
		//minQc = ptr->data->QuantumCounter;
		if(minQc == 0)
		{
			ptr->data->TurnAroundCounter = ptr->data->TurnAroundCounter-1;
			ptr3->data->ResponseTimeCounter = ptr3->data->ResponseTimeCounter-1;
			ptr3->data->QuantumCounter = ptr3->data->QuantumCounter+1;
			ptr3->data->status = 1; 
			currThread  = dequeue(ptr3->data);
			return;
		}

		if((minQc == -1  && ptr2->data->status!=2) || (minQc>ptr2->data->QuantumCounter && ptr2->data->status!=2))
		{
			minQc = ptr2->data->QuantumCounter;
			ptr3 = ptr2;
		}
		ptr2 = ptr2->next;

	}
	if(minQc != -1)
	{
		ptr3->data->QuantumCounter = ptr3->data->QuantumCounter+1;
		ptr->data->TurnAroundCounter = ptr->data->TurnAroundCounter-1;
		ptr3->data->ResponseTimeCounter = ptr3->data->ResponseTimeCounter-1;
		ptr3->data->status = 1;
		currThread = dequeue(ptr3->data);
		//swapcontext(&schedulerctx, &tcbPtr->context);

	}
	else
	{
		printf("Everything is blocked.");
	}
	//Need to turn the thread of ptr3;


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
void enqueue(tcb *thread){//insert tcb at end of runqueue
//make new node and then add thread to node->data

	thread->status = 0;
	if(runqueuehead == NULL)
	{
		runqueuehead->data = thread;
		runqueuehead->next = NULL;
		return;
	}
		struct Node *newNode = malloc(sizeof(struct Node));	
		newNode->data = thread;
		newNode->next = NULL;

		struct Node *ptr = runqueuehead;

		while(ptr->next != NULL){
			ptr = ptr->next;
		}

		ptr->next = newNode;

		if(thread->QuantumCounter == 0 && thread->ResponseTimeCounter !=-1 )
		{
			avg_resp_time = thread->ResponseTimeCounter*Quantum; 
		}
		else
		{
			thread->ResponseTimeCounter = -1;
		}

}

tcb* dequeue(tcb* thread){//delete node with specific thread tcb
//return tcb to caller func 

	tot_cntx_switches++;
	struct Node *ptr = runqueuehead;
	struct Node *ptr2 = runqueuehead;

	if(ptr == NULL)
	{
		return NULL; 
	}
	if(ptr->data == thread)
	{
		runqueuehead = runqueuehead->next;
		free(ptr);
		//free(ptr2);
		return thread;
	}

	while(1)
	{
		ptr2=ptr2->next;
		if(ptr2->data == thread)
		{
			ptr = ptr2->next;
			free(ptr2);
			return thread;
		}
		ptr = ptr->next;
	}


}


void handler(int signum){//signal handler

swapcontext(&(currThread->context), &schedulerctx);

}

/* How MLFQ would look visually example */
/*	(highest priority)
*	Q0 T1 Main	(time slice: 10ms)
*	Q1 T3		(time slice: 20ms)
*	Q2		(time slice: 30ms)
*	Q3 T2		(time slice: 40ms)
*	(lowest priority)
*	T3 is given a maximum of 20ms to run since in Q1 and in its tcb will have to keep a counter to see how many time quantums it has run. 
*	If T3 uses up its time quantum, meanining while it is in Q1, it ended up running for a total of 20ms, even if it ran for 10ms once then yielded and then ran for another 10ms, it will
*	get moved down to a lower queue (Q2). This concept will happen for all tcbs(threads) except for main (id = 0). After some designated time quantum (S), all currently queued tcbs(threads)
*	will get moved to Q0 and policy continues. 
*
*	How to check if tcb gave up CPU before time slice
*
*	Using T3 as example, could be the case where T3 is changed to its status RUNNING and it called worker_yield only after 10ms, 
*	where its status would be changed to READY, and then it swapcontext 
*	to the scheduler, where the scheduler checks if the tcb(thread) has ran for a full time slice of its queue (Q1 -> 20ms) and it would see that it only ran for 10ms.
* 	Then, the scheduler would usually allow for the thread to run for another 10ms (based on timer) where it would have completed its time slice and then be moved down a queue (Q2),
*	but it sees that T3 status is set to READY or !RUNNING, where it knows that T3 gave up CPU, so it would then instead be kept at Q1 with its time slice counter at 10ms (or 1 quantum).
* 	Scheduler would then enqueue T3 in Q1 and look for next tcb(thread) to run, and when encounter T3 again, T3 would run and then timer go off at 10ms, time slice complete, move T3 
* 	down to Q2 and reset counter for time slice to 0.
*/
