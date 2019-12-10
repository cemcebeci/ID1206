#include <stdio.h>
#include "green.h"

int fib( int i ){
	if( i == 0)
		return 1;
	if( i == 1)
		return 1;
	return fib( i -1) + fib( i - 2);
}

void *test(void * arg) {
	int id = *(int *)arg;
	for(int i = 0; i < 10000000; i++) {
		fib(21);
		printf("thread: %d, iteration: %d\n", id, i);
	}
}

int main() {

	green_t g0, g1;
	int a0 = 0, a1 = 1;
	
	green_create(&g0, test, &a0);
	green_create(&g1, test, &a1);

	green_join(&g0, NULL);
	green_join(&g1, NULL);
	printf("done, timercount: %d\n", timercount);
	return 0;
}