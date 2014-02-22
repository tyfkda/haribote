.globl	api_fopen
.include "syscall.def"

# int api_fopen(const char* fname)
api_fopen:
	push	%ebx
	mov	8(%esp), %ebx	# fname
	syscall	API_FOPEN
	pop	%ebx
	ret
