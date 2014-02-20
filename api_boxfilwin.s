.globl	api_boxfilwin

# void api_boxfilwin(int win, int x0, int y0, int x1, int y1, int col)
api_boxfilwin:
	push	%edi
	push	%esi
	push	%ebp
	push	%ebx
	mov	$7, %edx
	mov	20(%esp), %ebx	# win
	mov	24(%esp), %eax	# x0
	mov	28(%esp), %ecx	# y0
	mov	32(%esp), %esi	# x1
	mov	36(%esp), %edi	# y1
	mov	40(%esp), %ebp	# col
	int	$0x40
	pop	%ebx
	pop	%ebp
	pop	%esi
	pop	%edi
	ret
