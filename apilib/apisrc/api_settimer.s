.globl	api_settimer
.include "syscall.def"

# void api_settimer(TIMER* timer, int time)
api_settimer:
	mov	4(%esp), %ecx	# timer
	mov	8(%esp), %eax	# time
	syscall	API_SETTIMER
	ret
