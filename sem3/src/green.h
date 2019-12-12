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

typedef struct green_cond_t {
	green_t *thread;
	struct green_cond_t *next;
} green_cond_t;

typedef struct green_mutex_node {
	green_t *thread;
	struct green_mutex_node *next;
}green_mutex_node;

typedef struct green_mutex_t {
	volatile int taken;
	struct green_mutex_node *head;
} green_mutex_t;


int green_create( green_t *thread, void *(*fun)(void *), void *arg);
int green_yield();
int green_join(green_t *thread, void **val);

void green_cond_init( green_cond_t *cond);
int green_cond_wait( green_cond_t *cond, green_mutex_t *mutex);
void green_cond_signal( green_cond_t *cond);

int green_mutex_init( green_mutex_t *mutex);
int green_mutex_lock( green_mutex_t *mutex);
int green_mutex_unlock( green_mutex_t *mutex);

void report();

int timercount;