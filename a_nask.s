.globl	api_putchar, api_putstr0, api_end

# void api_putchar(int c)
api_putchar:
	mov	$1, %edx
	mov	4(%esp), %eax	# c
	int	$0x40
	ret

# void api_putstr0(const char* s)
api_putstr0:
	push	%ebx
	mov	$2, %edx
	mov	8(%esp), %ebx	# s
	int	$0x40
	pop	%ebx
	ret

# void api_end(void)
api_end:
	mov	$4, %edx
	int	$0x40
