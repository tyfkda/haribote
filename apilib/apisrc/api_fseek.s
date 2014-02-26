.globl	api_fseek
.include "syscall.def"

# void api_fseek(int fhandle, int offset, int mode)
api_fseek:
	push	%ebx
	mov	8(%esp), %eax	# fhandle
	mov	12(%esp), %ebx	# offset
	mov	16(%esp), %ecx	# mode
	syscall	API_FSEEK
	pop	%ebx
	ret
