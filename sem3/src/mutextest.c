#include <stdio.h>
#include "green.h"

typedef struct argstr
{
	int id;
	green_mutex_t *mutex;
} argstr;

int count = 0;

void *increment( void *arg) {
	argstr *argument = (argstr *) arg;
	int id = argument->id;
	green_mutex_t *mutex = argument->mutex;
	for(int i = 0; i < 1000000; i++) {
		green_mutex_lock(mutex);
		count++;
		green_mutex_unlock(mutex);
		printf("id: %d i incremented to: %d\n", id, count);
	}
}

int main() {
	green_t t0, t1;
	green_mutex_t mutex;
	argstr arg0, arg1;
	int id0 = 0, id1 = 1;

	green_mutex_init(&mutex);
	arg0.id = id0;
	arg1.id = id1;
	arg0.mutex = &mutex;
	arg1.mutex = &mutex;

	green_create(&t0, increment, &arg0);
	green_create(&t1, increment, &arg1);

	green_join(&t0, NULL);
	green_join(&t1, NULL);

	printf("done, timercount: %d\ncount: %d\n", timercount, count);
	return 0;

}