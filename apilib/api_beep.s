.globl	api_beep
.include "syscall.def"

# void api_beep(int tone)
api_beep:
	mov	4(%esp), %eax	# tone
	syscall	API_BEEP
	ret
