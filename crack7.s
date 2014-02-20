.globl	HariMain

HariMain:
	mov	$4, %ax			# 1005 * 8 for GDT
	mov	%ax, %ds
	cmpl	$0x69726148, %ds:(0x0004)	# Confirm ID (Hari)
	jne	fin			# Not application

	mov	%ds:(0x0000), %ecx	# Size of data segment
	mov	$12, %ax		# 2005 * 8 for GDT
	mov	%ax, %ds

crackloop:
	add	$-1, %ecx
	movb	$123, %ds:(%ecx)
	cmp	$0, %ecx
	jne	crackloop
fin:
	mov	$4, %edx
	int	$0x40
