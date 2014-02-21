.globl	api_closewin

# void api_closewin(int win)
api_closewin:
	push	%ebx
	mov	$14, %edx
	mov	8(%esp), %ebx	# win
	int	$0x40
	pop	%ebx
	ret
