.globl	api_closewin
.include "syscall.def"

# void api_closewin(int win)
api_closewin:
	mov	4(%esp), %eax	# win
	syscall	API_CLOSEWIN
	ret
