.globl	api_settimer
.include "syscall.def"

# void api_settimer(TIMER* timer, int time)
api_settimer:
	push	%ebx
	mov	8(%esp), %ebx	# timer
	mov	12(%esp), %eax	# time
	syscall	API_SETTIMER
	pop	%ebx
	ret
