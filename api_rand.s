.globl	rand

# int rand(void)
rand:
	push	%edi
	push	%esi
	mov	$10001, %edx
	int	$0x40
	pop	%esi
	pop	%edi
	ret
