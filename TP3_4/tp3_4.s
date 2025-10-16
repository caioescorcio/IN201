/* ======== Question 2 ======== */
.global enter_coroutine /* Makes enter_coroutine visible to the linker */
enter_coroutine:
	mov %rdi,%rsp /* RDI contains the argument to enter_coroutine. */
						/* And is copied to RSP. */
	pop %rbp
	pop %rbx
	pop %r12
	pop %r13
	pop %r14
	pop %r15
	/* 'ret' reads the next 8 bytes at [rsp] into %rip (the instruction pointer).*/
	ret /* Pop the program counter and them goes to the current RSP*/ 

/* ======== Question 5 ======== */
.global switch_coroutine /* Makes switch_coroutine visible to the linker */
switch_coroutine:
	push %r15
	push %r14
	push %r13
	push %r12
	push %rbx
	push %rbp
	
	mov %rsp,(%rdi) /* Store the stack pointer to *(first argument) */
	mov %rsi,%rdi
	jmp enter_coroutine /* Call enter_coroutine with the second argument */

/* Tell LD we don't need an executable stack here */
.section .note.GNU-stack,"",@progbits
