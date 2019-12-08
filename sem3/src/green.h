#include <ucontext.h>

typedef struct green_t {
	ucontext_t *context;
	void *(*fun)(void *); 	//start routine
	void *arg; 				//start routine arguments
	struct green_t *next;	//for the linked list
	struct green_t *join;	//the thread waiitng for the current thread to finish
	void *retval;			//return value of the current thread
	int zombie;				//whether or not the current thread is finished
} green_t;

int green_create( green_t *thread, void *(*fun)(void *), void *arg);
int green_yield();
int green_join(green_t *thread, void **val);
void report();