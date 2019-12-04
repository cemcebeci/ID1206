#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "rand.h"
#include "ptmall.h"

#define BUFFER 100
#define ROUNDS 1
#define LOOP 1000

int main(int argc, char **argv){
	srand ( time( NULL));
	int bufsize = atoi(argv[1]);
	int loop = atoi(argv[2]);

	void *buffer[bufsize];
	for (int i = 0; i < bufsize; i++){
		buffer[i] = NULL;
	}

	void *init = sbrk(0);
	void *current;


	for(int j = 0; j < ROUNDS; j++){
		int fcnt = 0;
		int acnt = 0;
		for(int i = 0; i < loop; i++){
			int index = rand() % bufsize;
			if ( buffer[index] != NULL){
				pree( buffer[index]);
				buffer[index] = NULL;
				fcnt++;				
			} else{
				size_t size = (size_t)request();
				int *memory;
				memory = palloc(size);

				if(memory == NULL) {
					printf("memory allocation failed\n");
					return(1);
				}
				buffer[index] = memory;
				*memory = 123;
				acnt++;
			}	
		}
		int *counts;
		int free;
		int counts_size = 100;
		counts = (int *) calloc(counts_size, sizeof(int));
		int length = flist_data(counts, &free);
		FILE *fp = fopen("dist.dat", "w");

		printf("allocation count: %d\n", acnt);
		printf("free count: %d\n", fcnt);
		printf("freelist length: %d\n", length);
		printf("total free space: %d\n", free);
		printf("average free block length: %d\n", (free / length));
		for( int k = 0; k < counts_size; k ++){
			fprintf( fp, "%d %d\n", 8 * k, counts[k]);
		}

		fclose(fp);
	}
	return 0;
}