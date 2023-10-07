#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <ucontext.h>

#define STACK_SIZE SIGSTKSZ

ucontext_t footx,bartx;

void* foo(ucontext_t *bartx){
    while(1){
        puts("foo\n");
    }
}

void* bar(ucontext_t *footx){
    while(1){
        puts("bar\n");
    }
}

void ring(int signum){
	printf("Switching contexts\n");

}


int main(int argc, char **argv){
    

    if (argc != 1) {
		printf(": USAGE Program Name and no Arguments expected\n");
		exit(1);
	}

    // Use sigaction to register signal handler
	struct sigaction sa;
	memset (&sa, 0, sizeof (sa));
	sa.sa_handler = &ring;
	sigaction (SIGPROF, &sa, NULL);

	// Create timer struct
	struct itimerval timer;

	// Set up what the timer should reset to after the timer goes off
	timer.it_interval.tv_usec = 0; 
	timer.it_interval.tv_sec = 5;

    timer.it_value.tv_usec = 0;
	timer.it_value.tv_sec = 1;

    void *foostack=malloc(STACK_SIZE);
    void *barstack = malloc(STACK_SIZE);

    /* Setup context that we are going to use */
	footx.uc_link=NULL;
	footx.uc_stack.ss_sp=foostack;
	footx.uc_stack.ss_size=STACK_SIZE;
	footx.uc_stack.ss_flags=0;

    bartx.uc_link=NULL;
	bartx.uc_stack.ss_sp=barstack;
	bartx.uc_stack.ss_size=STACK_SIZE;
	bartx.uc_stack.ss_flags=0;

    makecontext(&footx, (void*)&foo,1,&bartx);
    makecontext(&bartx, (void*)&bar,1,&footx);

    // Set the timer up (start the timer)
	setitimer(ITIMER_PROF, &timer, NULL);

    while(1);

    return 0;
}
