.globl	api_putstr0

# void api_putstr0(const char* s)
api_putstr0:
	push	%ebx
	mov	$2, %edx
	mov	8(%esp), %ebx	# s
	int	$0x40
	pop	%ebx
	ret
