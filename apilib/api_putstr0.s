.globl	api_putstr0
.include "syscall.def"

# void api_putstr0(const char* s)
api_putstr0:
	push	%ebx
	mov	8(%esp), %ebx	# s
	syscall	API_PUTSTR0
	pop	%ebx
	ret
