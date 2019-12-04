#include <stdio.h>
#include "rand.h"

int main(){
	int hist[256];
	for(int i = 0; i < 256; i++)
		hist[i] = 0;
	FILE *f = fopen("grph.dat","w");
	for(int i = 0; i < 1000000; i++){
		hist[request()]++;
	}
	for(int i = 0; i < 256; i++)
		fprintf(f,"%d %d\n",i,hist[i]);
}