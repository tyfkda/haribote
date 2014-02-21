.globl	api_point

# void api_point(int win, int x, int y, int col)
api_point:
	push	%edi
	push	%esi
	push	%ebx
	mov	$11, %edx
	mov	16(%esp), %ebx	# win
	mov	20(%esp), %esi	# x
	mov	24(%esp), %edi	# y
	mov	28(%esp), %eax	# col
	int	$0x40
	pop	%ebx
	pop	%esi
	pop	%edi
	ret
