#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "rand.h"

#define BUFFER 100
#define ROUNDS 10
#define LOOP 100000

int main(){

	void *init = sbrk(0);
	void *current;
	printf("The initial top of the heap: %p\n", init);

	for(int j = 0; j < ROUNDS; j++){
		for(int i = 0; i < LOOP; i++){
			size_t size = (request() % 4000) + sizeof(int);
			int *memory;
			memory = malloc(size);
			if(memory == NULL){
				fprintf(stderr, "malloc failed\n");
				return(1);
			} 

			*memory = 123;

			
			free(memory);
		}
		current = sbrk(0);
		int allocated = (int)((current - init) / 1024);
		printf("%d\n",j);
		printf("The current top of the heap is: %p\n",current);
		printf("	increased by %d Kbyte \n", allocated);
	}
	return 0;
}