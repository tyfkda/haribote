.globl	main
	.type	main, @function
main:
	mov	$2, %edx	# putstr0
	mov	$msg, %ebx
	int	$0x40
	mov	$0, %eax
	ret

	.section	.rodata
msg:
	.ascii	"hello\n"
	.byte	0
