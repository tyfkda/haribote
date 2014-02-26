.globl	api_putstr1
.include "syscall.def"

# void api_putstr1(const char* s, int l)
api_putstr1:
	push	%ebx
	mov	8(%esp), %ebx	# s
	mov	12(%esp), %ecx	# l
	syscall	API_PUTSTR1
	pop	%ebx
	ret
