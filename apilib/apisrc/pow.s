.globl	pow

pow:
	push	%ebp
	mov	%esp, %ebp
	sub	$4, %esp
	push	%ebx

	fstcw	-2(%ebp)
	orw	$0b110000000000, -2(%ebp)
	fldcw	-2(%ebp)

	fldl	16(%ebp)	# b
	fldl	8(%ebp)		# a
	fyl2x			# x = b * log2(a)

	fld	%st(0)		# [x, x]
	frndint			# [int(x), x]
	fld	%st(0)		# [int(x), int(x), x]
	fxch	%st(2)		# [x, int(x), int(x)]
	fsubp			# [frac(x), int(x)]

	f2xm1			# Calculate exponential 2 ^ frac(x) - 1
	fld1
	faddp			# [2 ^ frac(x), int(x)]
	fscale			# [2 ^ int(x) * 2 ^ frac(x)]
	fstp	%st(1)

	pop	%ebx
	mov	%ebp, %esp
	pop	%ebp
	ret
