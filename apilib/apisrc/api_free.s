.globl	api_free
.include "syscall.def"

# void api_free(void* addr, int size)
api_free:
	push	%ebx
	mov	%cs:(0x20), %ebx	# Address of memman for the task
	mov	8(%esp), %ecx		# addr
	mov	12(%esp), %ecx		# Size
	syscall	API_FREE
	pop	%ebx
	ret
