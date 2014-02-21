.globl	HariMain
	.type	HariMain, @function
HariMain:
	mov	$2, %edx	# putstr0
	mov	$msg, %ebx
	int	$0x40
	mov	$4, %edx
	int	$0x40

	.section	.rodata
msg:
	.ascii	"hello\n"
	.byte	0
