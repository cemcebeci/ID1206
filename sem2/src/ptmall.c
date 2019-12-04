#include <sys/mman.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

struct head {
	uint16_t bfree;
	uint16_t bsize;
	uint16_t free;
	uint16_t size;
	struct head *next;
	struct head *prev;
};

#define TRUE 1
#define FALSE 0

#define HEAD (sizeof(struct head))
#define MIN(size) ((size) > 8) ? (size) : 8
#define LIMIT(size) (MIN(0) + HEAD + size)

#define MAGIC(memory) ((struct head *)memory -1)
#define HIDE(block) (void *) ((struct head *)block + 1)
#define ALIGN 8
#define ARENA (64*1024)
#define EXTRA 10 * 8

struct head *after (struct head *block){
	return (struct head*) ( (char *)block + block->size + HEAD);
}

struct head *before (struct head *block){
	return (struct head*) ( (char *)block - block->bsize - HEAD);
}

struct head *extras[EXTRA / 8];
struct head *flist = NULL; // the free list, only one for now.

void detach(struct head *block){
	if( block->next != NULL){
		block->next->prev = block->prev;
	}

	if(block->prev != NULL){
		block->prev->next = block->next;
	} else {
		if(block->size <= EXTRA){
			int index = ( block->size / 8) - 1;
			extras[index] = block->next;
			return;
		}
		flist = block->next;
	}
}

void insert(struct head *block) {
	if(block->size <= EXTRA){
		int index = ( block->size / 8) - 1;
		block->next = extras[index];
		block->prev = NULL;
		if( extras[index] != NULL)
			extras[index]->prev = block;
		extras[index] = block;
		return;
	}
	block->next = flist;
	block->prev = NULL;
	if( flist != NULL)
		flist->prev = block;
	flist = block;
}

struct head *split(struct head *block, int size){
	int rsize = block->size - size - HEAD;	
	int min = MIN(rsize);

	if(rsize < min){ // SIKIYIM BOYLE DILI, SIKEYIM DILINIZI
		printf("The block is too small to be split with the specified size");
		return NULL;
	}
	detach(block);

	block->size = size;

	struct head *splt = (struct head*)((char *)block + HEAD + block->size);
	splt->bsize = size;
	splt->bfree = block->free;
	splt->size = rsize;
	splt->free = TRUE; // CAUTION: I'm not that sure, depends on who has access to the split method.

	struct head* aft = (struct head *)((char *)splt + splt->size + HEAD);
	aft->bsize = splt->size;
	return splt;
}

struct head *arena = NULL;

struct head *new(){
	if(arena != NULL){
		printf("one arena already allocated \n");
		return NULL;
	}

	for(int i = 0; i < EXTRA / 8; i++){
		extras[i] = NULL;
	}

	struct head *new = mmap(NULL, ARENA, PROT_READ | PROT_WRITE,
											MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if(new == MAP_FAILED) {
		printf("mmap failed");
	}

	uint16_t size = ARENA - 2 * HEAD;

	new->bfree = FALSE;
	new->bsize = -1; //completely arbitrary, might need to change.
	new->free = TRUE;
	new->size = size;

	struct head *sentinel = after(new);
	sentinel->bfree = -1;
	sentinel->bsize = -1;
	sentinel->free = FALSE; //??????
	sentinel->size = -1;

	arena = (struct head*) new;
	insert((struct head *)new);//!!
	return new;
}

int adjust( int req){
	if (req % ALIGN == 0)
		return req;
	else
		return req + ALIGN - (req % ALIGN);
}

struct head *find( uint16_t size){
	
	if( size <= EXTRA){
		int index = (size / 8) - 1;
		if( extras[index] != NULL)
			return extras[index];
	}

	//should find a block of at least specified size and split it if it is suitable
	for (struct head *cur = flist; cur != NULL; cur = cur->next ) {
		if( cur->size >= size){
			//found!
			if(cur->size >= LIMIT(size)){
				struct head* new = split( cur, size);
				insert(cur);
				insert(new);
			}
			return cur;
		}
	}
	printf("Insufficient memory\n");
	return NULL;
}

void *palloc(size_t request) {
	//ADDED BY ME

	if(arena == NULL)
		arena = new();

	if(request <=0){
		return NULL;
	}

	int size = adjust(request);

	struct head *taken = find(size);

	if(taken == NULL)
		return NULL;

	else {
		taken->free = FALSE;
		after(taken)->bfree = FALSE;
		detach(taken);
		return HIDE(taken);
	}
}

struct head *merge(struct head *block) {
	struct head *aft = after(block);

	int recurse = FALSE;

	if(block->bfree){
		detach(before(block));

		int newsize = block->size + before(block)->size + HEAD;
		before(block)->size = newsize;
		after(block)->bsize = newsize;
		block = before(block);

		recurse = TRUE;
	}

	if(aft->free){
		detach(aft);

		int newsize = aft->size + block->size + HEAD;
		block->size = newsize;
		after(aft)->bsize = newsize;

		recurse = TRUE;
	}
	if(recurse)
		return merge(block);
	else
		return block;
}

void pree( void *memory){
	if( memory != NULL){
		struct head *block = MAGIC(memory);
		block = merge(block);
		block->free = TRUE;
		struct head *aft = after(block);
		aft->bfree = TRUE;
		insert(block);
	}
	return;
}



int flist_data( int *counts, int *free){
	*free = 0;
	int count = 0;
	for(struct head* cur = flist; cur != NULL;
		cur = cur->next, count ++){
		counts[cur->size / 8]++;
		*free += cur->size;
	}
	for(int i = 0; i < EXTRA / 8; i++){
		for(struct head* cur = extras[i]; cur != NULL;
			cur = cur->next, count ++){
			counts[cur->size / 8]++;
			*free += cur->size;
		}
	}
	return count;
}