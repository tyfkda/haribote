.globl	api_now
.include "syscall.def"

# int api_now(unsigned char* buf)
api_now:
	mov	4(%esp), %eax	# buf
	syscall	API_NOW
	ret
