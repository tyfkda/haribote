.globl	api_point
.include "syscall.def"

# void api_point(int win, int x, int y, int col)
api_point:
	push	%edi
	push	%esi
	mov	12(%esp), %ecx	# win
	mov	16(%esp), %esi	# x
	mov	20(%esp), %edi	# y
	mov	24(%esp), %eax	# col
	syscall	API_POINT
	pop	%esi
	pop	%edi
	ret
