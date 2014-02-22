.globl	api_fread
.include "syscall.def"

# int api_fread(void* buf, int maxsize, int fhandle)
api_fread:
	push	%ebx
	mov	8(%esp), %ebx	# buf
	mov	12(%esp), %ecx	# maxsize
	mov	16(%esp), %eax	# fhandle
	syscall	API_FREAD
	pop	%ebx
	ret
