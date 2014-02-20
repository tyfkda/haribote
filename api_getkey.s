.globl	api_getkey

# int api_getkey(int mode)
api_getkey:
	mov	$15, %edx
	mov	4(%esp), %eax	# mode
	int	$0x40
	ret
