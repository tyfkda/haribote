.globl	api_freetimer
.include "syscall.def"

# void api_freetimer(TIMER* timer)
api_freetimer:
	mov	4(%esp), %eax	# timer
	syscall	API_FREETIMER
	ret
