.globl	sin
	
# double sin(double x)
sin:
	fldl	4(%esp)		# x
	fsin
	ret
