.globl	api_alloctimer

# TIMER* api_alloctimer(void)
api_alloctimer:
	mov	$16, %edx
	int	$0x40
	ret
