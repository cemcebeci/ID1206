#include <stdlib.h>
#include <stdio.h>
#include <ucontext.h>
#include <assert.h>
#include <signal.h>
#include <sys/time.h>
#include "green.h"


#define FALSE 0
#define TRUE 1

#define STACK_SIZE 4096

#define PERIOD 100 //period for the timer interrupt

static ucontext_t main_cntx = {0};
static green_t main_green = {&main_cntx, NULL, NULL, NULL, NULL, NULL, FALSE};

static green_t *running = &main_green;
static green_t *end = &main_green;

static sigset_t block;

void timer_handler(int);

static void init() __attribute__((constructor));

int critic = FALSE; // for the timer interrupt

void init() {
	getcontext(&main_cntx);

	sigemptyset(&block);
	sigaddset(&block, SIGVTALRM);
	struct sigaction act = {0};
	struct timeval interval;
	struct itimerval period;

	act.sa_handler = timer_handler;
	assert(sigaction(SIGVTALRM, &act, NULL) == 0);

	interval.tv_sec = 0;
	interval.tv_usec = PERIOD;
	period.it_interval = interval;
	period.it_value = interval;
	setitimer(ITIMER_VIRTUAL, &period, NULL);
}


int timercount = 0;
void timer_handler( int sig) {
	if(critic)
		return;
	timercount++;
	green_yield();
}
void report(){
	for(green_t *cur = running; cur != NULL; cur = cur->next)
		printf("thread at: %p\n", cur);
}

void green_thread() {// IMPORTANT NOTE! The type of result is assumed in here,
					 // check that!
	green_t *this = running;

	void **result = (*this->fun)(this->arg);

	//place the waiting thread in the ready queue
	if( this->join != NULL) {
		end->next = this->join;
		end = this->join;
		end->next = NULL;
	}

	this->retval = result;
	this->zombie = TRUE;
	
	green_t *next = running->next;
	running = next;

	setcontext(next->context); //This also calls the green_thread function.
}

int green_create( green_t *new, void *(*fun)( void *), void *arg) {
	ucontext_t *cntx = (ucontext_t *)malloc( sizeof(ucontext_t));
	getcontext(cntx);

	void *stack = malloc(STACK_SIZE);

	cntx->uc_stack.ss_sp = stack;
	cntx->uc_stack.ss_size = STACK_SIZE;
	makecontext(cntx, green_thread, 0);

	new->context = cntx;
	new->fun = fun;
	new->arg = arg;
	new->next = NULL; // Redundant, consider removing this.
	new->join = NULL;
	new->retval = NULL;
	new->zombie = FALSE;
	end->next = new;
	end = new;
	end->next = NULL;
}

int green_yield() {
	
	sigprocmask( SIG_BLOCK, &block, NULL);
	green_t *susp = running;
	end->next = susp;
	end = susp;
	green_t *next = running->next;
	susp->next = NULL;
	running = next;
	sigprocmask( SIG_UNBLOCK, &block, NULL);
	swapcontext(susp->context, running->context);
	return 0;
}

int green_join(green_t *thread, void **res){

	if( !thread->zombie){
		sigprocmask( SIG_BLOCK, &block, NULL);
		green_t *susp = running;
		thread->join = susp;
		green_t *next = running->next;
		running = next;
		sigprocmask( SIG_UNBLOCK, &block, NULL);
		swapcontext(susp->context, running->context);
	}
	//This is where the waiting thread's execution will continue
	if(res != NULL)
		*res = thread->retval;
	free(thread->context);
	return 0;
}

//--------------------------Conditinal Variables-------------------------

void green_cond_init( green_cond_t *cond){
	cond->thread = NULL;
	cond->next = NULL;
}

void green_cond_wait( green_cond_t *cond){
	sigprocmask( SIG_BLOCK, &block, NULL);
	green_t *this = running;

	//Add a new node to the start of the list
	green_cond_t *new = (green_cond_t *)malloc(sizeof(green_cond_t));
	new->thread = this;
	new->next = cond->next;
	cond->next = new;

	//detach the current thread from the ready queue and switch thread
	green_t *next = running->next;
	running = next;
	sigprocmask( SIG_UNBLOCK, &block, NULL);
	swapcontext(this->context, running->context);
}

void green_cond_signal( green_cond_t *cond){
	if(cond->next == NULL)
		return;
	sigprocmask( SIG_BLOCK, &block, NULL);
	//move the thread to the ready queue (to the end for now).
	green_t *thr = cond->next->thread;
	end->next = thr;
	end = thr;
	thr->next = NULL; 

	//detach the node from the conditional list and deallocate
	green_cond_t *old = cond->next;
	cond->next =  old->next;
	sigprocmask( SIG_BLOCK, &block, NULL);
	free(old);
}

//-----------------------------Mutex----------------------------------

int green_mutex_init(green_mutex_t *mutex) {
	mutex->taken = FALSE;
	mutex->head = NULL;
	return 0;
}

int green_mutex_lock(green_mutex_t *mutex) {
	sigprocmask( SIG_BLOCK, &block, NULL);

	if(mutex->taken) {
		green_t *this = running;

		//add the current thread to the waiting list.
		green_mutex_node *new = (green_mutex_node *)malloc(sizeof(green_mutex_node));
		new->next = mutex->head;
		mutex->head = new;
		new->thread = this;

		green_t *next = running->next;
		running = next;
		swapcontext(this->context, running->context);
	} else {
		mutex->taken = TRUE;

	}
	sigprocmask( SIG_UNBLOCK, &block, NULL);
	return 0;
}

int green_mutex_unlock( green_mutex_t *mutex) {
	sigprocmask( SIG_BLOCK, &block, NULL);
	if(mutex->head != NULL) {
		green_t *next = mutex->head->thread;

		//remove the head from the waiting list
		green_mutex_node *temp = mutex->head->next;
		free(mutex->head);
		mutex->head = temp;

		//add the head to the ready queue
		end->next = next;
		end = next;
		next->next = NULL;
	} else{
		mutex->taken = FALSE;
	}
	sigprocmask( SIG_UNBLOCK, &block, NULL);
	return 0;
}