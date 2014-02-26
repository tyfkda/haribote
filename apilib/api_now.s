.globl	api_now
.include "syscall.def"

# int api_now(unsigned char* buf)
api_now:
	push	%ebx
	mov	8(%esp), %ebx	# buf
	syscall	API_NOW
	pop	%ebx
	ret
