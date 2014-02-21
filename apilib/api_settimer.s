.globl	api_settimer

# void api_settimer(TIMER* timer, int time)
api_settimer:
	push	%ebx
	mov	$18, %edx
	mov	8(%esp), %ebx	# timer
	mov	12(%esp), %eax	# time
	int	$0x40
	pop	%ebx
	ret
