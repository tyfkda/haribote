.globl	api_beep

# void api_beep(int tone)
api_beep:
	mov	$20, %edx
	mov	4(%esp), %eax	# tone
	int	$0x40
	ret
