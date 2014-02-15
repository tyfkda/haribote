.globl	api_putchar, api_putstr0, api_end, api_openwin, api_closewin
.globl	api_putstrwin, api_boxfilwin, api_point, api_refresh, api_linewin
.globl	api_getkey
.globl	api_initmalloc, api_malloc, api_free
.globl	api_alloctimer, api_inittimer, api_settimer, api_freetimer
.globl	api_dumphex, rand, sprintf

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

# void api_initmalloc(void)
api_initmalloc:
	push	%ebx
	mov	$8, %edx
	mov	%cs:(0x20), %ebx	# Address of malloc start
	mov	%ebx, %eax
	add	$32 * 1024, %eax	# Add 32KB
	mov	%cs:(0x00), %ecx	# Size of data segment
	sub	%eax, %ecx
	int	$0x40
	pop	%ebx
	ret

# void* api_malloc(int size)
api_malloc:
	push	%ebx
	mov	$9, %edx
	mov	%cs:(0x20), %ebx	# Address of memman for the task
	mov	8(%esp), %ecx		# Size
	int	$0x40
	pop	%ebx
	ret

# void api_free(void* addr, int size)
api_free:
	push	%ebx
	mov	$10, %edx
	mov	%cs:(0x20), %ebx	# Address of memman for the task
	mov	8(%esp), %ecx		# addr
	mov	12(%esp), %ecx		# Size
	int	$0x40
	pop	%ebx
	ret

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

# void api_refresh(int win, int x0, int y0, int x1, int y1)
api_refresh:
	push	%edi
	push	%esi
	push	%ebx
	mov	$12, %edx
	mov	16(%esp), %ebx	# win
	mov	20(%esp), %eax	# x0
	mov	24(%esp), %ecx	# y0
	mov	28(%esp), %esi	# x1
	mov	32(%esp), %edi	# y1
	int	$0x40
	pop	%ebx
	pop	%esi
	pop	%edi
	ret

# void api_linewin(int win, int x0, int y0, int x1, int y1, int col)
api_linewin:
	push	%edi
	push	%esi
	push	%ebp
	push	%ebx
	mov	$13, %edx
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

# void api_closewin(int win)
api_closewin:
	push	%ebx
	mov	$14, %edx
	mov	8(%esp), %ebx	# win
	int	$0x40
	pop	%ebx
	ret

# int api_getkey(int mode)
api_getkey:
	mov	$15, %edx
	mov	4(%esp), %eax	# mode
	int	$0x40
	ret

# TIMER* api_alloctimer(void)
api_alloctimer:
	mov	$16, %edx
	int	$0x40
	ret

# void api_inittimer(TIMER* timer, int data)
api_inittimer:
	push	%ebx
	mov	$17, %edx
	mov	8(%esp), %ebx	# timer
	mov	12(%esp), %eax	# data
	int	$0x40
	pop	%ebx
	ret

# void api_settimer(TIMER* timer, int time)
api_settimer:
	push	%ebx
	mov	$18, %edx
	mov	8(%esp), %ebx	# timer
	mov	12(%esp), %eax	# time
	int	$0x40
	pop	%ebx
	ret

# void api_freetimer(TIMER* timer)
api_freetimer:
	push	%ebx
	mov	$19, %edx
	mov	8(%esp), %ebx	# timer
	int	$0x40
	pop	%ebx
	ret

# void api_dumphex(int val)
api_dumphex:
	push	%edi
	push	%esi
	push	%ebx
	mov	$10000, %edx
	mov	16(%esp), %eax	# val
	int	$0x40
	pop	%ebx
	pop	%esi
	pop	%edi
	ret

# int rand(void)
rand:
	push	%edi
	push	%esi
	mov	$10001, %edx
	int	$0x40
	pop	%esi
	pop	%edi
	ret

# int sprintf(char* buf, const char* format, ...)
sprintf:
	push	%ebx
	push	%ecx
	push	%edi
	mov	16(%esp), %ebx	# buf
	mov	20(%esp), %ecx	# format
	lea	24(%esp), %edi	# va_list
	mov	$10002, %edx
	int	$0x40
	pop	%edi
	pop	%ecx
	pop	%ebx
	ret
