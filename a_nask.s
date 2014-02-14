.globl	api_putchar, api_end

# void api_putchar(int c)
api_putchar:
	mov	$1, %edx
	mov	4(%esp), %eax	# c
	int	$0x40
	ret

# void api_end(void)
api_end:
	mov	$4, %edx
	int	$0x40
