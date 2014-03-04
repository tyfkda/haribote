.globl	api_inittimer
.include "syscall.def"

# void api_inittimer(TIMER* timer, int data)
api_inittimer:
	mov	4(%esp), %ecx	# timer
	mov	8(%esp), %eax	# data
	syscall	API_INITTIMER
	ret
