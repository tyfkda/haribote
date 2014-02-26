.globl	api_closewin
.include "syscall.def"

# void api_closewin(int win)
api_closewin:
	push	%ebx
	mov	8(%esp), %ebx	# win
	syscall	API_CLOSEWIN
	pop	%ebx
	ret
