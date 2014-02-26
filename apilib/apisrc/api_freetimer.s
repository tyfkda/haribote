.globl	api_freetimer
.include "syscall.def"

# void api_freetimer(TIMER* timer)
api_freetimer:
	push	%ebx
	mov	8(%esp), %ebx	# timer
	syscall	API_FREETIMER
	pop	%ebx
	ret
