.globl	api_putchar, api_putstr0, api_end, api_openwin
.globl	api_putstrwin, api_boxfilwin

# void api_putchar(int c)
api_putchar:
	mov	$1, %edx
	mov	4(%esp), %eax	# c
	int	$0x40
	ret

# void api_putstr0(const char* s)
api_putstr0:
	push	%ebx
	mov	$2, %edx
	mov	8(%esp), %ebx	# s
	int	$0x40
	pop	%ebx
	ret

# void api_end(void)
api_end:
	mov	$4, %edx
	int	$0x40

# int api_openwin(unsigned char* buf, int xsiz, int ysiz, int col_inv, const char* title)
api_openwin:
	push	%edi
	push	%esi
	push	%ebx
	mov	$5, %edx
	mov	16(%esp), %ebx	# buf
	mov	20(%esp), %esi	# xsiz
	mov	24(%esp), %edi	# ysiz
	mov	28(%esp), %eax	# col_inv
	mov	32(%esp), %ecx	# title
	int	$0x40
	pop	%ebx
	pop	%esi
	pop	%edi
	ret

# void api_putstrwin(int win, int x, int y, int col, int len, const char* str)
api_putstrwin:
	push	%edi
	push	%esi
	push	%ebp
	push	%ebx
	mov	$6, %edx
	mov	20(%esp), %ebx	# win
	mov	24(%esp), %esi	# x
	mov	28(%esp), %edi	# y
	mov	32(%esp), %eax	# col
	mov	36(%esp), %ecx	# len
	mov	40(%esp), %ebp	# str
	int	$0x40
	pop	%ebx
	pop	%ebp
	pop	%esi
	pop	%edi
	ret

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
