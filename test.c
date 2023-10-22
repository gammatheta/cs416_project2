#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "../thread-worker.h"

/* A scratch program template on which to call and
 * test thread-worker library functions as you implement
 * them.
 *
 * You can modify and use this program as much as possible.
 * This will not be graded.
 */

pthread_mutex_t mutex;
pthread_t thread1;
pthread_t thread2;
int counter1;
int counter2;
int sum = 0;

void checker(void *arg){

	for(int i = 0; i < 300; i++){
		pthread_mutex_lock(&mutex);
		sum++;
		pthread_mutex_unlock(&mutex);	
	}

	printf("Exiting thread\n");
	pthread_exit(NULL);

}

void sig_handler(int signum){
	printf("%d\n",signum);
}

int main(int argc, char **argv) {

	/* Implement HERE */

	printf("This is tester code\n");

	printf("Initialize mutex\n");
	pthread_mutex_init(&mutex,NULL);

	printf("Create threads\n");
	pthread_create(&thread1,NULL,&checker,&counter1);
	// printf("Made thread 1\n");
	pthread_create(&thread2,NULL,&checker,&counter2);
	// printf("Made thread 2\n");

	signal(SIGABRT, sig_handler);
	signal(SIGSEGV, sig_handler);

	printf("Join threads\n");
	pthread_join(thread1,NULL);
	// printf("Joined thread 1\n");
	pthread_join(thread2,NULL);
	// printf("Joined thread 2\n");

	printf("Destroy mutex\n");
	pthread_mutex_destroy(&mutex);

#ifdef USE_WORKERS
	fprintf(stderr , "Total sum is: %d\n", sum);
	print_app_stats();
	fprintf(stderr, "***************************\n");
#endif

	return 0;
}
