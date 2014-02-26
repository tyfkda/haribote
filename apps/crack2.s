.globl	main

main:
	mov	$1 * 8, %eax
	mov	%ax, %ds
	movb	$0, (0x102600)
	mov	$0, %eax
	ret
