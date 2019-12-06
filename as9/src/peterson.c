#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

volatile int count = 0;

volatile int request[2] = {0,0};
volatile int turn = 0;

void lock( int id){
	request[id] = 1;
	int other = 1 - id; //since the two id's are 1 and 0
	turn = other;
	while(request[other] == 1 && turn == other) {}; //spin
}

void unlock( int id) {
	request[id] = 0;
}