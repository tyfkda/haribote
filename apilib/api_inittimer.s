.globl	api_inittimer
.include "syscall.def"

# void api_inittimer(TIMER* timer, int data)
api_inittimer:
	push	%ebx
	mov	8(%esp), %ebx	# timer
	mov	12(%esp), %eax	# data
	syscall	API_INITTIMER
	pop	%ebx
	ret
