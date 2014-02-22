.globl	putchar

# int putchar(int c)
putchar:
	mov	$1, %edx
	mov	4(%esp), %eax	# c
	int	$0x40
	ret
