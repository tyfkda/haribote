.globl	api_fopen

# int api_fopen(const char* fname)
api_fopen:
	push	%ebx
	mov	$21, %edx
	mov	8(%esp), %ebx	# fname
	int	$0x40
	pop	%ebx
	ret
