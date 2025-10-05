#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>  // getchar
#include <stdlib.h> // exit
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

// The size of our stacks
#define STACK_SIZE_FULL 4096
// #define NTRHEADS 3	// 2 function threads and a main scheduler thread
#define NTRHEADS 4	// 2 costumers, 1 producer and 1 scheduler

void print_str(char *str) {
	while (*str) {
		putchar((int)*str);
    ++str;
}
}

// print a integer in base 10 using putchar
void print_int(int x) {
  if (x < 0) {
	  putchar('-');
	  x = -x;
  }
  if (x == 0) {
    putchar('0');
    return;
}
int pos = 1;
while (x >= pos) {
	pos *= 10;
}
while (pos > 1) {
	pos /= 10;
    putchar('0' + (x / pos) % 10);
}
}




typedef void *coroutine_t;
typedef char mystack_t[STACK_SIZE_FULL];

// the caracteristics of a thread are: the routine pointer, the stack pointer and the status (metadata)
typedef struct mythread {
	coroutine_t routine;
	mystack_t *stack;
	bool ready;
} mythread;

mythread threads [NTRHEADS];	// all my threads
coroutine_t scheduler_cr;		// my coroutine to schedule the threads
int current_thread = -1;		// my current running thread


// our 4 piles' stacks
mystack_t s1;
mystack_t s2;
mystack_t s3;
mystack_t s4;


// we define the coroutines out of the main function to make it possible to reuse them in the functions (foo() and bar())
coroutine_t cA, cB, prod;	

// char variable for Partie 4
char letter;
// status variable for Partie 4	
bool full;

/* Leaves the current context and loads the registers and stack of CR. */
void enter_coroutine(coroutine_t cr);

/* Saves the current context in p_from, and switches into TO. */
void switch_coroutine(coroutine_t *p_from, coroutine_t to);

/* Initializes the stack and returns a coroutine such that, when entered,
it will begin execution at the address initial_pc. */
coroutine_t init_coroutine(void *stack_begin, size_t stack_size,
void (*initial_pc)());

void initialize_threads(struct mythread * thread_vector);
void scheduler();
void yield();

void foo();
void bar();

void producer();
void customer_A();
void customer_B();
	
int main() { 
	fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);	// for a non-blocking getchar() function

	full = false;
	prod = init_coroutine(s1, STACK_SIZE_FULL, producer);
	cA = init_coroutine(s2, STACK_SIZE_FULL, customer_A);	
	cB = init_coroutine(s3, STACK_SIZE_FULL, customer_B);	

	scheduler_cr = init_coroutine(s4, STACK_SIZE_FULL, scheduler);
	
	threads[0].routine = prod;
	threads[0].stack   = &s1;
	
	threads[1].routine = cA;
	threads[1].stack   = &s2;

	threads[2].routine = cB;
	threads[2].stack   = &s3;

	initialize_threads(threads);
	enter_coroutine(scheduler_cr);

	return 0; 
}

// the init_coroutine function gets a function pointer and stack it's arguments
// when entering a coroutine, and uses it's RSP to get the initial address to the routine
// then it pops it's arguments to use them in the function execution (this is done in the ".s" file) 
coroutine_t init_coroutine(void *stack_begin, size_t stack_size, void (*initial_pc)()){
	char *stack_end = ((char *)stack_begin) + stack_size;
	void **ptr = (void**)stack_end;
	// initialize all the pointer's values as 0
	*(--ptr) = (void*)initial_pc;	// RSP to the function
	*(--ptr) = 0;   // r15
	*(--ptr) = 0;   // r14
	*(--ptr) = 0;   // r13
	*(--ptr) = 0;   // r12
	*(--ptr) = 0;   // rbx
	*(--ptr) = 0;   // rbp
	
	return (coroutine_t) ptr;
}

void initialize_threads(mythread * thread_vector){
	for (int i = 0; i < NTRHEADS -1; i++){
		thread_vector[i].ready = true;			
	}
	return;
}

void scheduler(){
	while (1){
		current_thread = (current_thread + 1) % NTRHEADS; 	// go through all the threads
		if(threads[current_thread].ready){ // if the thread is ready, it will change the context to the coroutine execution
			switch_coroutine(&scheduler_cr, threads[current_thread].routine);	
		}
	}
	
}

// changes the contexto to the scheduler
void yield(){
	switch_coroutine(&threads[current_thread].routine, scheduler_cr);
}


void foo(void) {
    int counter = 0;
    while (1) {
		if(getchar() != -1) counter = 0;	// if recieves keyboard input, resets counter
        print_str("FOO counter = ");
		print_int(counter++);
		print_str("\n");
        yield();				// at the end of each cycle, they will yield to change back to the scheduler
    }
}

void bar(void) {
	int counter = 0;
    while (1) {
        print_str("BAR counter = ");
		print_int(counter++);
		print_str("\n");
        yield();
    }
}

void producer(void) {
    while (1) {
        int ch = getchar();
        if (ch != -1 && ch != EOF) {      // something was typed
            if (!full) {                  // only write if slot is empty
                letter = (char)ch;
                full = true;
                print_str("Producer wrote: ");
                putchar(letter);
                print_str("\n");
            }
        }
        yield(); // give up CPU
    }
}

void customer_A(void) {
    while (1) {
        if (full) {
            full = false;
            print_str("Customer A printing: ");
			putchar(letter);
			print_str("\n");
        }
		yield();
    }
}

void customer_B(void) {
    while (1) {
        if (full) {
            full = false;
            print_str("Customer B printing: ");
			putchar(letter);
			print_str("\n");
        }
		yield();
    }
}