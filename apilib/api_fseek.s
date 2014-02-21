.globl	api_fseek

# void api_fseek(int fhandle, int offset, int mode)
api_fseek:
	push	%ebx
	mov	$23, %edx
	mov	8(%esp), %eax	# fhandle
	mov	12(%esp), %ebx	# offset
	mov	16(%esp), %ecx	# mode
	int	$0x40
	pop	%ebx
	ret
