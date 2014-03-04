.globl	api_putstr0
.include "syscall.def"

# void api_putstr0(const char* s)
api_putstr0:
	mov	4(%esp), %eax	# s
	syscall	API_PUTSTR0
	ret
