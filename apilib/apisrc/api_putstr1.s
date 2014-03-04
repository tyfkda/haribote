.globl	api_putstr1
.include "syscall.def"

# void api_putstr1(const char* s, int l)
api_putstr1:
	mov	8(%esp), %eax	# s
	mov	12(%esp), %ecx	# l
	syscall	API_PUTSTR1
	ret
