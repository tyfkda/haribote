.globl	api_freetimer

# void api_freetimer(TIMER* timer)
api_freetimer:
	push	%ebx
	mov	$19, %edx
	mov	8(%esp), %ebx	# timer
	int	$0x40
	pop	%ebx
	ret
