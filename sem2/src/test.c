#include <stdlib.h>
#include <stdio.h>
#include "ptmall.h"

int main(){
	int *a = (int*)palloc(1024);
	printf("%p\n", a);

	pree(a);

	a = (int*)palloc(1024);
	printf("%p\n", a);
	int *b = (int*)palloc(1024);
	printf("%p\n", b);

	pree(a);

	int *c = (int*)palloc(1025);
	printf("%p\n", c);
	return 0;
}