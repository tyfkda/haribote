.globl	api_openwin
.include "syscall.def"

# int api_openwin(unsigned char* buf, int xsiz, int ysiz, int col_inv, const char* title)
api_openwin:
	push	%edi
	push	%esi
	push	%ebx
	mov	16(%esp), %ebx	# buf
	mov	20(%esp), %esi	# xsiz
	mov	24(%esp), %edi	# ysiz
	mov	28(%esp), %eax	# col_inv
	mov	32(%esp), %ecx	# title
	syscall	API_OPENWIN
	pop	%ebx
	pop	%esi
	pop	%edi
	ret
