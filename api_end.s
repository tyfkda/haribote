.globl	api_end

# void api_end(void)
api_end:
	mov	$4, %edx
	int	$0x40
