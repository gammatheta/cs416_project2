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
uint idcounter = 1; //maintain unique ids for all threads
ucontext_t schedulerctx; //scheduler contexts
void *schedstack; //scheduler context stack
tcb *main_tcb;	//main tcb
enum boolean fstrun = true; //first run global variable
struct Node *runqueuehead; //PSJF runqueue head pointer
tcb *currThread; //current running thread
enum boolean finishedThreads[MAXTHREADS]; //manages which thread has finished (true) and which are not (false)
struct Node* mlfq[NUM_QUEUES]; //array of queues
double tot_turn_time = 0; //add total of all threads' turnaround time
double tot_resp_time = 0; //add total of all threads' response time 
int tot_thread_fin = 0; //add total of all threads that have finished
worker_mutex_t *my_mutex;

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

	    for(int i = 0; i < MAXTHREADS; i++){
			finishedThreads[i] = false;
		}

		tcb *newthread = malloc(sizeof(tcb));
		newthread->id = idcounter;
		thread = &(newthread->id); //unsure if works
		idcounter++;
		newthread->priority = 0;
		newthread->status = READY;
		newthread->quantumCounter = 0;
		newthread->fstsched = false;
		// newthread->turnAroundCounter = 0;
		// newthread->responseTimeCounter = 0;
		newthread->timeRan = 0;
		
		if (getcontext(&(newthread->context)) < 0){
			perror("getcontext");
			exit(1);
		}

		if(getcontext(&schedulerctx) < 0){
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

		main_tcb = malloc(sizeof(tcb));
		main_tcb->id = 0;
		main_tcb->priority = 0;
		main_tcb->status = READY;
		main_tcb->quantumCounter = 0;
		// main->turnAroundCounter = 0;
		// main->responseTimeCounter = 0;
		main_tcb->timeRan = 0;

		if (getcontext(&(main_tcb->context)) < 0){
			perror("getcontext");
			exit(1);
		}

		//Set up scheduler
		//swapcontext(&mainctx, &schedulerctx); //after swapcontext returns from scheduler, will go to next line
		//schedule();
		swapcontext(&(main_tcb->context), &schedulerctx);

		/* Setup context that we are going to use */
		newthread->stack = malloc(STACK_SIZE);

		(newthread->context).uc_link= &schedulerctx; //when thread finishes, will return to scheduler context
		(newthread->context).uc_stack.ss_sp= newthread->stack;
		(newthread->context).uc_stack.ss_size=STACK_SIZE;
		(newthread->context).uc_stack.ss_flags=0;

		makecontext(&(newthread->context), (void*)function, 1, arg);

		/* Will have to change since enqueue operation will be different for MLFQ and PSFJ so will be better to correctly enqueue 
		*  within scheduler or differentiate enqueue based on sched policy 
		*/
		//add tcb to runqueue

		/* Gets current time and places inside struct (second param) */
		//struct timespec start;
		//clock_gettime(CLOCK_REALTIME, &start);		
		clock_gettime(CLOCK_REALTIME,&(newthread->arrivetime));

		if(PSJF){
			enqueue(newthread);
			enqueue(main_tcb);
		}else{
			menqueue(newthread);
			menqueue(main_tcb);
		}
		

		fstrun = false;
		//switch to scheduler context and run thread based on scheduling protocol
		//continue building runqueue or regular execution

		swapcontext(&(main_tcb->context), &schedulerctx);


	   }else{//if not the first run of worker_create function
		tcb *newthread = malloc(sizeof(tcb));
		newthread->id = idcounter;
		thread = &(newthread->id); //unsure if works
		idcounter++;
		newthread->priority = 0;
		newthread->status = READY;
		newthread->quantumCounter = 0;
		newthread->fstsched = false;
		// newthread->turnAroundCounter = 0;
		// newthread->responseTimeCounter = 0;
		newthread->timeRan = 0;
		
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
		clock_gettime(CLOCK_REALTIME,&(newthread->arrivetime));
		if(PSJF){
			enqueue(newthread);
		}else{
			menqueue(newthread);
		}
	
		//switch to scheduler context and run thread based on scheduling protocol
		//continue building runqueue or regular execution

		swapcontext(&(main_tcb->context), &schedulerctx);
	   }
	
    return 0;
};

/* give CPU possession to other user-level worker threads voluntarily */
int worker_yield() {
	
	// - change worker thread's state from Running to Ready
	// - save context of this thread to its thread control block
	// - switch from thread context to scheduler context

	// YOUR CODE HERE

	//assuming current running thread is correctly designated
	currThread->status = READY;
	swapcontext(&(currThread->context), &schedulerctx);
	return 0;
};

/* terminate a thread */
void worker_exit(void *value_ptr) {
	// - de-allocate any dynamic memory created when starting this thread

	// YOUR CODE HERE
	
	//assuming current running thread is correctly designated
	//mark which thread finished
	uint id = currThread->id;

	if(currThread->id == main_tcb->id) return;//if main is exiting, then just return to main
	//maybe instead of return, swapcontext(&(currThread->context), &schedulerctx);

	finishedThreads[id] = true;
	tot_thread_fin++;


	clock_gettime(CLOCK_REALTIME,&(currThread->fintime));

	//collect any info about turnaround, response, or quantum counter before de-alloc

	//free tcb and stack of tcb
	free(currThread->stack);
	free(currThread);
	currThread = NULL;
	
};


/* Wait for thread termination */
int worker_join(worker_t thread, void **value_ptr) {
	
	// - wait for a specific thread to terminate
	// - de-allocate any dynamic memory created by the joining thread
  
	// YOUR CODE HERE

	/* currThread is waiting for thread with thread id (worker_t thread) to finish */
	//use finishedThreads to see if finished and if not then swap back to scheduler

	uint id = thread;

	if(id == main_tcb->id) return 0;// main trying to join itself
	//maybe instead of return, swapcontext(&(currThread->context), &schedulerctx);

	while(finishedThreads[id] != true){//while param thread not finished, yield CPU since don't need to continue running
		int waiting = worker_yield();
	}

	//param thread finished and can exit out of calling (currThread) thread
	worker_exit(NULL);

	return 0;
};

/* initialize the mutex lock */
int worker_mutex_init(worker_mutex_t *mutex, 
                          const pthread_mutexattr_t *mutexattr) {
	//- initialize data structures for this mutex

	// YOUR CODE HERE
	
	my_mutex = malloc(sizeof(worker_mutex_t));
	mutex = my_mutex;
	mutex->lock = false;
    mutex->thread = NULL;
	mutex->mutexQueueHead = NULL;
 	// mutex->mutexQueneHead = malloc(sizeof(struct Node));
	// mutex->mutexQueneHead->data = NULL;

	return 0;
};

/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex) {

        // - use the built-in test-and-set atomic function to test the mutex
        // - if the mutex is acquired successfully, enter the critical section
        // - if acquiring mutex fails, push current thread into block list and
        // context switch to the scheduler thread

        // YOUR CODE HERE
		/*
In C, the _sync_lock_test_and_set (or __sync_lock_test_and_set in some compilers) is a built-in function used for atomic operations on variables, 
primarily in the context of multi-threaded programming to ensure that shared data is accessed safely. 
This function is often used in low-level programming when 
you need to perform atomic operations without relying on higher-level synchronization mechanisms like mutexes or semaphores.

Here mutex->lock will automatically be set to false. 
*/
		enum boolean check = __sync_lock_test_and_set(&(mutex->lock), true);

		if(check == false){
			//mutex->lock = true
			mutex->thread = currThread;

		}else{//check was true; lock is previosly held by another thread
			//enqueue currThread on mutex queue and NOT back on sched queue
			//swap back to sched context
			struct Node *newNode = malloc(sizeof(struct Node));
			newNode->data = currThread;
			newNode->next = NULL;
			struct Node *ptr =  mutex->mutexQueueHead;

			while(ptr->next != NULL){
				ptr = ptr->next;
			}

			ptr->next = newNode;

			currThread->status = READY;

			currThread = NULL;

			swapcontext(&(newNode->data->context), &schedulerctx);
		}
	
        return 0;
};

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex) {
	// - release mutex and make it available again. 
	// - put threads in block list to run queue 
	// so that they could compete for mutex later.

	// YOUR CODE HERE
	__sync_lock_release(&(mutex->lock));

	//check if anything in queue
	//if so put first thread in mutex queue in sched queue
	if(mutex->mutexQueueHead != NULL){
		struct Node *ptr = mutex->mutexQueueHead;
		tcb *waitingThread = mutex->mutexQueueHead->data;
		mutex->mutexQueueHead = mutex->mutexQueueHead->next;
		free(ptr);

		if(PSJF){//if PSJF policy

			enqueue(waitingThread);
		
		}else{//MLFQ policy
			menqueue(waitingThread);
		}
	}

	mutex->thread = NULL;	

	return 0;
};


/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex) {
	// - de-allocate dynamic memory created in worker_mutex_init

	//check if mutex is currently locked and if so send -1
	if(mutex->lock == true) return -1;

	if(mutex->thread != NULL || mutex->mutexQueueHead != NULL) return -1;

	free(my_mutex);

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

		swapcontext(&schedulerctx, &(main_tcb->context));;

		//swapcontext(&schedulerctx, &mainctx); //return back to worker_create func
	}

		if(PSJF){
			// while(runqueuehead != NULL){
				while(1){
				sched_psjf();
				if(currThread->fstsched == false){
					clock_gettime(CLOCK_REALTIME,&(currThread->firstsched));
					currThread->fstsched = true;
				}
				swapcontext(&schedulerctx, &currThread->context);
				if(currThread!=NULL)
				{
					enqueue(currThread);
				}
			}
		}
		else{
			while(1){
				sched_mlfq();

			// int timeslice = 0;

			// timeslice = (thread->priority + 1) * QUANTUM; //for Q0, time slice is 10ms or 1 Quantum

			// if(thread->timeRan >= timeslice){
			// 	if(thread->priority != (NUM_QUEUES -1)){
			// 		thread->priority = thread->priority + 1;
			// 	}
			// }
			}
		}

	//fstrun = true; //if scheduler finished

}

/* Pre-emptive Shortest Job First (POLICY_PSJF) scheduling algorithm */
static void sched_psjf() {
	// - your own implementation of PSJF
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
	struct Node* ptr = runqueuehead;
	struct Node* ptr2 = runqueuehead;
	struct Node* ptr3 = runqueuehead;
	while(ptr!=NULL)
	{
		
		// ptr->data->turnAroundCounter = ptr->data->turnAroundCounter+1;
		// ptr->data->responseTimeCounter = ptr->data->responseTimeCounter+1;
		ptr = ptr->next;
	}
	//ptr = runquenehead; 

	int minQc = -1;
	if(ptr2->data->status != BLOCKED)
	{
		 minQc = ptr2->data->quantumCounter;
	}

	ptr2 = ptr2->next;
	while(ptr2!=NULL)
	{
		//minQc = ptr->data->quantumCounter;
		if(minQc == 0)
		{
			// ptr->data->turnAroundCounter = ptr->data->turnAroundCounter-1;
			// ptr3->data->responseTimeCounter = ptr3->data->responseTimeCounter-1;
			ptr3->data->quantumCounter = ptr3->data->quantumCounter+1;
			ptr3->data->status = RUNNING; 
			currThread  = dequeue(ptr3->data->id);
			printf("currThread is id %d\n", currThread->id);
			return;
		}

		if((minQc == -1  && ptr2->data->status!=BLOCKED) || (minQc>ptr2->data->quantumCounter && ptr2->data->status!=BLOCKED))
		{
			minQc = ptr2->data->quantumCounter;
			ptr3 = ptr2;
		}
		ptr2 = ptr2->next;

	}
	if(minQc != -1)
	{
		ptr3->data->quantumCounter = ptr3->data->quantumCounter+1;
		// ptr->data->turnAroundCounter = ptr->data->turnAroundCounter-1;
		// ptr3->data->responseTimeCounter = ptr3->data->responseTimeCounter-1;
		ptr3->data->status = RUNNING;
		currThread = dequeue(ptr3->data->id);
		printf("currThread is id %d\n", currThread->id);
		//swapcontext(&schedulerctx, &tcbPtr->context);

	}
	else
	{
		printf("Everything is blocked.");
	}
	//Need to turn the thread of ptr3;
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
	thread->status = READY;
	if(runqueuehead == NULL)
	{
		struct Node *start = malloc(sizeof(struct Node));
		start->data = thread;
		start->next = NULL;
		runqueuehead = start;
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

}

tcb* dequeue(worker_t threadid){//delete node with specific thread tcb
//return tcb to caller func 
	tot_cntx_switches++;
	struct Node *ptr = runqueuehead;
	struct Node *ptr2 = runqueuehead;

	tcb *returnthread;

	if(ptr == NULL)
	{
		return NULL; 
	}
	if(ptr->data->id == threadid)
	{
		runqueuehead = runqueuehead->next;
		returnthread = ptr->data;
		free(ptr);
		//free(ptr2);
		return returnthread;
	}

	while(1)
	{
		ptr2=ptr2->next;
		if(ptr2->data->id == threadid)
		{
			ptr->next = ptr2->next;
			returnthread = ptr2->data;
			free(ptr2);
			return returnthread;
		}
		ptr = ptr->next;
	}

}

void menqueue(tcb *thread){ //enqueue for MLFQ
	//assume priority level for thread is final

	struct Node *ptr = mlfq[thread->priority]; //pointer to head of specific queue in mlfq

	thread->status = READY;
	if(ptr == NULL)
	{
		struct Node *start = malloc(sizeof(struct Node));
		start->data = thread;
		start->next = NULL;
		mlfq[thread->priority] = start;
		return;
	}
		struct Node *newNode = malloc(sizeof(struct Node));	
		newNode->data = thread;
		newNode->next = NULL;

		struct Node *temp = mlfq[thread->priority];

		while(temp->next != NULL){
			temp = temp->next;
		}

		temp->next = newNode;

}

tcb* mdequeue(tcb *thread){ //dequeue for MLFQ
	//return tcb to caller func 
	tot_cntx_switches++;
	struct Node *ptr = mlfq[thread->priority];
	struct Node *ptr2 = mlfq[thread->priority];
	struct Node *temp;

	tcb *returnthread;

	if(ptr == NULL)
	{
		return NULL; 
	}
	if(ptr->data == thread)
	{
		temp = ptr;
		mlfq[thread->priority] = mlfq[thread->priority]->next;
		returnthread = ptr->data;
		free(temp);
		return returnthread;
	}

	while(1)
	{
		ptr2=ptr2->next;
		if(ptr2->data == thread)
		{
			ptr->next = ptr2->next;
			returnthread = ptr2->data;
			free(ptr2);
			return returnthread;
		}
		ptr = ptr->next;
	}
}

/*void mutexEnqueue(mutex *mutex){//insert tcb at end of runqueue
//make new node and then add thread to node->data

	if(mutexQueneHead == NULL)
	{
		mutexQueneHead->data = mutex;
		mutexQueneHead->next = NULL;
		return;
	}
		struct mutexNode *newmutexNode = malloc(sizeof(struct mutexNode));	
		mutexNode->data = mutex;
		mutexNode->next = NULL;

		struct mutexNode *ptr = mutexQueneHead;

		while(ptr->next != NULL){
			ptr = ptr->next;
		}

		ptr->next = newmutexNode;

}

void mutexDequeue(tcb* mutex){//delete node with specific thread tcb
//return tcb to caller func 

	struct mutexNode *ptr = mutexQueneHead;
	struct mutexNode *ptr2 = mutexQueneHead;

	if(ptr == NULL)
	{
		return NULL; 
	}
	if(ptr->data == mutex)
	{
		mutexQueneHead = mutexQueneHead->next;
		enqueue(ptr->mutex->data->thread);
		free(ptr);
		//free(ptr2);
		//return thread;
	}

	while(1)
	{
		ptr2=ptr2->next;
		if(ptr2->data == thread)
		{
			ptr = ptr2->next;
			enqueue(ptr2->mutex->data->thread);
			free(ptr2);
			//return thread;
		}
		ptr = ptr->next;
	}


}*/


void handler(int signum){//signal handler
	swapcontext(&(currThread->context),&schedulerctx);

}
