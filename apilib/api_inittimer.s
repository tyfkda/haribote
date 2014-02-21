.globl	api_inittimer

# void api_inittimer(TIMER* timer, int data)
api_inittimer:
	push	%ebx
	mov	$17, %edx
	mov	8(%esp), %ebx	# timer
	mov	12(%esp), %eax	# data
	int	$0x40
	pop	%ebx
	ret
