.globl	api_fwrite
.include "syscall.def"

# int api_fwrite(void* buf, int maxsize, int fhandle)
api_fwrite:
	push	%ebx
	mov	8(%esp), %ebx	# buf
	mov	12(%esp), %ecx	# maxsize
	mov	16(%esp), %eax	# fhandle
	syscall	API_FWRITE
	pop	%ebx
	ret
