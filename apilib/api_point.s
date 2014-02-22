.globl	api_point
.include "syscall.def"

# void api_point(int win, int x, int y, int col)
api_point:
	push	%edi
	push	%esi
	push	%ebx
	mov	16(%esp), %ebx	# win
	mov	20(%esp), %esi	# x
	mov	24(%esp), %edi	# y
	mov	28(%esp), %eax	# col
	syscall	API_POINT
	pop	%ebx
	pop	%esi
	pop	%edi
	ret
