.globl	api_fread

# int api_fread(void* buf, int maxsize, int fhandle)
api_fread:
	push	%ebx
	mov	$25, %edx
	mov	8(%esp), %ebx	# buf
	mov	12(%esp), %ecx	# maxsize
	mov	16(%esp), %eax	# fhandle
	int	$0x40
	pop	%ebx
	ret
