.globl	api_cmdline
.include "syscall.def"

# int api_cmdline(char* buf, int maxsize)
api_cmdline:
	push	%ebx
	mov	8(%esp), %ebx	# buf
	mov	12(%esp), %ecx	# maxsize
	syscall	API_CMDLINE
	pop	%ebx
	ret
