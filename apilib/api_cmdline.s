.globl	api_cmdline

# int api_cmdline(char* buf, int maxsize)
api_cmdline:
	push	%ebx
	mov	$26, %edx
	mov	8(%esp), %ebx	# buf
	mov	12(%esp), %ecx	# maxsize
	int	$0x40
	pop	%ebx
	ret
