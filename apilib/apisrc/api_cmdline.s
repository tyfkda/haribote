.globl	api_cmdline
.include "syscall.def"

# int api_cmdline(char* buf, int maxsize)
api_cmdline:
	mov	4(%esp), %ecx	# buf
	mov	8(%esp), %eax	# maxsize
	syscall	API_CMDLINE
	ret
