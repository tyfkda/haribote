.globl	api_alloctimer
.include "syscall.def"

# TIMER* api_alloctimer(void)
api_alloctimer:
	syscall	API_ALLOCTIMER
	ret
