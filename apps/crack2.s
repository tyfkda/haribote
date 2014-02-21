.globl	HariMain

HariMain:
	mov	$1 * 8, %eax
	mov	%ax, %ds
	movb	$0, (0x102600)
	mov	$4, %edx
	int	$0x40
