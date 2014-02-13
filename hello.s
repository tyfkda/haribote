.globl	start

start:
	mov	$2, %edx	# putstr0
	mov	$msg, %ebx
	int	$0x40
	retf
msg:
	.ascii	"hello\n"
	.byte	0
