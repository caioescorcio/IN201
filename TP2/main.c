#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#define MEM_SIZE 16000
#define HEAP_SIZE 16000

// we define a chained list to go through our heap
typedef struct block {
    int size;              // size of this free block (excluding header) (the WHOLE size)
    struct block *next;    // pointer to next free block
} mem_block;

// for creating the heap space using the memory allocation, it needs to have the memory space to do so
static char heap[HEAP_SIZE];            // then we use the "fake heap" as a memory for our heap
static mem_block *free_list = NULL;     // we start our chained list as null   


#ifdef partie1

char mem[MEM_SIZE];    // 16 Kilo-octet allocation
int top = 0;        // variable to the top of the memory

void *memalloc(int size) {
    if (top + size >= MEM_SIZE) return NULL;
    char * ret_addr = &mem[top];
    top += size;
    return ret_addr;
}
#endif

// for allocating the memory in the heap it's necessary to initialize our heap. We do so by starting a chained list in
// the memory dedicated for "heap"
void meminit(void){
    free_list = (mem_block *) heap;

    free_list->size = HEAP_SIZE - sizeof(mem_block);
    free_list->next = NULL;
}


//#ifdef partie2
// for memalloc, it needs to bring allocate the memory in our "free space", to do so we start by verifying if our size is 
// divisible by 8, to allocate as octets
void *memalloc(int size){   
    
    if (size % 8 != 0) size += (8 - (size % 8));
    // then we start our pointers to go through our free memory (one to have the current block and other to have the previous block)
    mem_block * previous = NULL;
    mem_block * current = free_list;    // we start at the current free list address

    // now we go through the list verifying if there is any space in the blocks that satisfy the required size 
    while (current != NULL){
        // if there is a nice size, we allocate
        if (current->size >= size){     // if there is enough memory to allocate, we try it 

            if(current->size <= size + (int)sizeof(mem_block)){     // we verify if there is enough space for both the allocated memory and for the "next block" pointer
                // if yes, we REMOVE the whole block from the "FREE space", returning only a pointer to the current space
                // it will save the previous "next" block to the free list
                if (previous == NULL) free_list = current->next;          // the new free list is the current block of free memory
                // if the previous block is not null, it means that we have to address the previous block (the previous free list) to continue having it's next block. 
                // view heap_2.png
                else previous->next = current->next;     // the new "previous value" is going to be the current memory block
                
                return (char *)current + sizeof(mem_block);     // it return the pointer to the current block (with the desired size) with an offset of 1 memory block OUTSIDE THE LIST
            }
    
            // if there not enough space in one block, we will go through the new split the memory
            // the allocation for the new_block is used only to guarantee its manipulation
            mem_block *new_block = (mem_block *)((char *)current + sizeof(mem_block) + size);   // a new block of memory with the current mem_block pointer, another mem_block and the size
            new_block->size = current->size - size - sizeof(mem_block);         // new FREE memory size, without the other space
            new_block->next = current->next;
            // same thing as last time
            if (previous == NULL) free_list = new_block;
            else previous->next = new_block;   
    
            current->size = size;
            return (char *)current + sizeof(mem_block);
        }
        previous = current;
        current = current->next;
    }
    return NULL;
}
//#endif

// for this function it's necessary to pick the begin of the chained list and
// to 
void memfree_without_fusion(void *ptr) {
    if (ptr == NULL) return;    // checks if the pointer has anything to supress
    // we receive "(char *)current + sizeof(mem_block)", but we want only the pointer "current"
    mem_block * unallocated_block = (mem_block*) ((char *)ptr - sizeof(mem_block)); // we get the pointer to the begin of the block
    // now we put the current block at the begin of our list, it's size doesnt change (but now it's an empty space)
    unallocated_block->next = free_list;    // now we put the unallocated block in the begin of the empty space
    free_list = unallocated_block;  // and now it is the new list
    return;
}

// it prevents the situation where there are 2 adjacent free blocks and one after another and we would like to add then in only one memory
void memfree(void *ptr){
    if (ptr == NULL) return;
    mem_block * unallocated_block = (mem_block*) ((char *)ptr - sizeof(mem_block)); // we get the pointer to the begin of the block
    
    mem_block **current = &free_list;   // pointer to pointer to the free list ("copies it")
    // this loop moves current forward in the free list until we find the correct position where unallocated_block should be inserted while keeping the list sorted by memory address.
    while (*current && *current < unallocated_block) {  // finds the address after the address of the now unallocated block in the free space (where it was previously)
        current = &(*current)->next;
    }

    // when found it is necessary to add the memory fields
    unallocated_block->next = *current;     // equivalent to "free_list = unallocated block -> list "
    *current = unallocated_block;

    // unallocated_block->next  ---------------------------------------------------------------------- verifies if after the current block there is another block
    // (char *)unallocated_block + sizeof(mem_block) + unallocated_block->size ----------------------- verifies if the pointer after the current block + its size + the given size is equivalent to the pointer to the next address
    // it means that  [pointer + allocated_memory + memory_block] = [next_pointer] : memories are adjacent 
    if (unallocated_block->next && (char *)unallocated_block + sizeof(mem_block) + unallocated_block->size == (char *)unallocated_block->next) {
        unallocated_block->size += sizeof(mem_block) + unallocated_block->next->size;   // now we allocate the new size to the first block
        unallocated_block->next = unallocated_block->next->next;    // we point the old "next" to the next "next"
    }
    
    return;
}

int main() {
    meminit();
    printf("free list = %p\n", free_list);  // %p = endereço absoluto de memória
    void *p1 = memalloc(10000);
    printf("free list = %p\n", free_list);
    void *p2 = memalloc(2000);
    printf("free list = %p\n", free_list);

    printf("p1 = %p\n", p1);
    printf("p2 = %p\n", p2);

    memfree(p1);
    memfree(p2);

    printf("free list = %p\n", free_list);
    return 0;
}
