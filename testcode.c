#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <ucontext.h>

#define STACK_SIZE SIGSTKSZ

int cswitch = 0;
//ucontext_t footx,bartx;

void foo(ucontext_t *footx, ucontext_t *bartx){
    while(1){
        printf("foo\n");
		if(cswitch){
			cswitch = 0;
			swapcontext(footx,bartx);
		}
    }
}

void bar(ucontext_t *footx, ucontext_t *bartx){
    while(1){
        printf("bar\n");
		if(cswitch){
			cswitch = 0;
			swapcontext(bartx,footx);
		}
    }
}

void ring(int signum){
	//printf("Switching contexts\n");

	cswitch = 1;

}


int main(int argc, char **argv){
    ucontext_t footx,bartx;

    if (argc != 1) {
		printf(": USAGE Program Name and no Arguments expected\n");
		exit(1);
	}

    if (getcontext(&footx) < 0){
		perror("getcontext");
		exit(1);
	}

	if (getcontext(&bartx) < 0){
		perror("getcontext");
		exit(1);
	}

	// // Use sigaction to register signal handler
	struct sigaction sa;
	memset(&sa, 0, sizeof (sa));
	sa.sa_handler = &ring;
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


    void *foostack = malloc(STACK_SIZE);
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
	
    makecontext(&footx, (void*)&foo,2,&footx,&bartx);
    makecontext(&bartx, (void*)&bar,2,&footx,&bartx);

    //swapcontext(&maintx,&footx);
	
	foo(&footx,&bartx);
	
	//Does not run
	puts("finished running");
	return 0;
    
}
