#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>  // getchar
#include <stdlib.h> // exit
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#define STACK_SIZE_FULL 4096	
// #define NTRHEADS 3	
#define NTRHEADS 4	
void print_str(char *str) {
	while (*str) {
		putchar((int)*str);
    ++str;
	}	
}

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

typedef char mystack_t[STACK_SIZE_FULL] __attribute__((aligned(4096)));	// thread stack type + memory alignment codition (for 4096 bytes stacks)

typedef struct mythread {
	coroutine_t routine;	
	mystack_t *stack;	
	bool ready;
} mythread;

mythread threads [NTRHEADS];	
coroutine_t scheduler_cr;		
int current_thread = -1;		


mystack_t s1;
mystack_t s2;
mystack_t s3;
mystack_t s4;

coroutine_t cA, cB, prod;	

char *debug;

char letter;
bool full;

void enter_coroutine(coroutine_t cr);

void switch_coroutine(coroutine_t *p_from, coroutine_t to);

coroutine_t init_coroutine(void *stack_begin, size_t stack_size,
void (*initial_pc)());

void initialize_threads(struct mythread * thread_vector);
void scheduler();
void yield();

void signal_handler(int signal);

void foo();
void bar();

void producer();
void customer_A();
void customer_B();
	

int main() { 
	signal(SIGSEGV, signal_handler);
	printf("Program start: \n\n\n\n");
	fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);	
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
	
	// memory protection field, it forbids each stack to be accessed at all
	mprotect(s1, 4096, PROT_NONE);	// PROT_NONE field guaratees that it wont be accessible
	mprotect(s2, 4096, PROT_NONE);
	mprotect(s3, 4096, PROT_NONE);
	print_str("Memory protection done: NONE OF THE STACKS CAN ACCESS ITS PILES\n\n\n\n");
	enter_coroutine(scheduler_cr);
	
	return 0; 
}

void signal_handler(int sig) {
    sigset_t set;
    sigfillset(&set);
    sigprocmask(SIG_UNBLOCK, &set, 0);

    printf("\nThread %d error\n", current_thread);

    switch (current_thread) {	// restarts the current thread
        case 0:
            threads[0].routine = init_coroutine(threads[0].stack, STACK_SIZE_FULL, producer);
            break;
        case 1:
            threads[1].routine = init_coroutine(threads[1].stack, STACK_SIZE_FULL, customer_A);
            break;
        case 2:
            threads[2].routine = init_coroutine(threads[2].stack, STACK_SIZE_FULL, customer_B);
            break;
        default:
            break;
    }

    
    threads[current_thread].ready = false;

    // Voltar para o scheduler
    switch_coroutine(&threads[current_thread].routine, scheduler_cr);
}

coroutine_t init_coroutine(void *stack_begin, size_t stack_size, void (*initial_pc)()){
	char *stack_end = ((char *)stack_begin) + stack_size;
	void **ptr = (void**)stack_end;

	*(--ptr) = (void*)initial_pc;	
	*(--ptr) = 0;   
	*(--ptr) = 0;   
	*(--ptr) = 0;   
	*(--ptr) = 0;   
	*(--ptr) = 0;   
	*(--ptr) = 0;   
	
	return (coroutine_t) ptr;
}

void initialize_threads(mythread * thread_vector){
	for (int i = 0; i < NTRHEADS -1; i++){
		thread_vector[i].ready = true;		
		printf("Stack address [%d]: %p\n", i+1, thread_vector[i].stack);
	}
	return;
}

void scheduler(){
	
	while (1){
		current_thread = (current_thread + 1) % NTRHEADS; 	
		if(threads[current_thread].ready){ 
			// before entering each thread it authorizes it's usage by enabling the corresponding PROTECTIONS for each thread
			mprotect(threads[current_thread].stack, 4096, PROT_READ | PROT_WRITE);
			switch_coroutine(&scheduler_cr, threads[current_thread].routine);
			mprotect(threads[current_thread].stack, 4096, PROT_NONE);	// in the end it 
		}
	}
	
}

void yield(){
	switch_coroutine(&threads[current_thread].routine, scheduler_cr);
}

void foo(void) {
    int counter = 0;
    while (1) {
		if(getchar() != -1) counter = 0;
        print_str("FOO counter = ");
		print_int(counter++);
		print_str("\n");
        yield();				
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
		if (!full) {                  
        	int ch = getchar();
        	if (ch != -1 && ch != EOF) {      
                letter = (char)ch;
                full = true;
                print_str("Producer wrote: ");
                putchar(letter);
                print_str("\n");

				// condition to verify if a function can't be accessed by other thread directly
				if ((char) ch == 'e'){
					*((char*) threads[1].stack) = 'X';	// tries to write in another thread's stack
				}
            }
        }
        yield(); 
    }
}

void customer_A(void) {
    while (1) {
        if (full) {
            full = false;
			int loop_letter = (int) letter;
			for(int i = 0; i < loop_letter; i++){
				print_str("Customer A printing: ");
				putchar((char) loop_letter);
				print_str("\n");
				yield();
			}
        }
		yield();
    }
}

void customer_B(void) {
    while (1) {
        if (full) {
            full = false;
            int loop_letter = (int) letter;
			for(int i = 0; i < loop_letter; i++){
				print_str("Customer B printing: ");
				putchar((char) loop_letter);
				print_str("\n");
				yield();
			}
        }
		yield();
    }
}