.globl	api_fopen
.include "syscall.def"

# int api_fopen(const char* fname, int flag)
api_fopen:
	push	%ebx
	mov	8(%esp), %ebx	# fname
	mov	12(%esp), %eax	# flag
	syscall	API_FOPEN
	pop	%ebx
	ret
