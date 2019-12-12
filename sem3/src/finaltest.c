#include <stdio.h>
#include "green.h"

int flag = 0;
green_mutex_t mutex;
green_cond_t cond;

void *test(void * arg) {
	int loop = 1000000;
	int id = *(int *)arg;

	while(loop > 0) {
		green_mutex_lock(&mutex);
		
		while(flag != id){
			green_cond_wait(&cond, &mutex);
		}
		flag = (id + 1) % 2;

		green_cond_signal(&cond);
		green_mutex_unlock(&mutex);
		loop --;
	}
}

int main() {

	green_t g0, g1;
	int a0 = 0, a1 = 1;
	green_mutex_init(&mutex);
	green_cond_init(&cond);
	green_create(&g0, test, &a0);
	green_create(&g1, test, &a1);


	green_join(&g0, NULL);
	green_join(&g1, NULL);
	printf("done, timercount: %d\n", timercount);
	return 0;
}