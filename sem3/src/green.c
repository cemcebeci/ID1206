#include <stdlib.h>
#include <stdio.h>
#include <ucontext.h>
#include <assert.h>
#include "green.h"

#define FALSE 0
#define TRUE 1

#define STACK_SIZE 4096

static ucontext_t main_cntx = {0};
static green_t main_green = {&main_cntx, NULL, NULL, NULL, NULL, NULL, FALSE};

static green_t *running = &main_green;
static green_t *end = &main_green;

static void init() __attribute__((constructor));

void init() {
	getcontext(&main_cntx);
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
	green_t *susp = running;
	end->next = susp;
	end = susp;
	green_t *next = running->next;
	susp->next = NULL;
	running = next;
	swapcontext(susp->context, next->context);
	return 0;
}

int green_join(green_t *thread, void **res){

	if( !thread->zombie){
		green_t *susp = running;
		thread->join = susp;
		green_t *next = running->next;
		running = next;
		swapcontext(susp->context, next->context);
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
	green_t *this = running;

	//Add a new node to the start of the list
	green_cond_t *new = (green_cond_t *)malloc(sizeof(green_cond_t));
	new->thread = this;
	new->next = cond->next;
	cond->next = new;

	//detach the current thread from the ready queue and switch thread
	green_t *next = running->next;
	running = next;
	swapcontext(this->context, next->context);
}

void green_cond_signal( green_cond_t *cond){
	if(cond->next == NULL)
		return;
	//move the thread to the ready queue (to the end for now).
	green_t *thr = cond->next->thread;
	end->next = thr;
	end = thr;
	thr->next = NULL; 

	//detach the node from the conditional list and deallocate
	green_cond_t *old = cond->next;
	cond->next =  old->next;
	free(old);
}