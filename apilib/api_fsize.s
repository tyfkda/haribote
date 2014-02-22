.globl	api_fsize
.include "syscall.def"

# int api_fsize(int fhandle, int mode)
api_fsize:
	mov	4(%esp), %eax	# fhandle
	mov	8(%esp), %ecx	# mode
	syscall	API_FSIZE
	ret
