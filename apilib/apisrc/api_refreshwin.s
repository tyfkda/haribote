.globl	api_refreshwin
.include "syscall.def"

# void api_refreshwin(int win, int x0, int y0, int x1, int y1)
api_refreshwin:
	push	%edi
	push	%esi
	push	%ebx
	mov	16(%esp), %ebx	# win
	mov	20(%esp), %eax	# x0
	mov	24(%esp), %ecx	# y0
	mov	28(%esp), %esi	# x1
	mov	32(%esp), %edi	# y1
	syscall	API_REFRESHWIN
	pop	%ebx
	pop	%esi
	pop	%edi
	ret
