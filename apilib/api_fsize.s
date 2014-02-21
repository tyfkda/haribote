.globl	api_fsize

# int api_fsize(int fhandle, int mode)
api_fsize:
	mov	$24, %edx
	mov	4(%esp), %eax	# fhandle
	mov	8(%esp), %ecx	# mode
	int	$0x40
	ret
