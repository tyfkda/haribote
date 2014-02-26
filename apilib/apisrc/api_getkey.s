.globl	api_getkey
.include "syscall.def"

# int api_getkey(int mode)
api_getkey:
	mov	4(%esp), %eax	# mode
	syscall	API_GETKEY
	ret
