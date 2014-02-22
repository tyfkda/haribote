.globl	api_end
.include "syscall.def"

# void api_end(void)
api_end:
	syscall	API_END
