.globl	api_putstr1

# void api_putstr1(const char* s, int l)
api_putstr1:
	push	%ebx
	mov	$3, %edx
	mov	8(%esp), %ebx	# s
	mov	12(%esp), %ecx	# l
	int	$0x40
	pop	%ebx
	ret
