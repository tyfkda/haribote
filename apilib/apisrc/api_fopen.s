.globl	api_fopen
.include "syscall.def"

# int api_fopen(const char* fname, int flag)
api_fopen:
	mov	4(%esp), %ecx	# fname
	mov	8(%esp), %eax	# flag
	syscall	API_FOPEN
	ret
