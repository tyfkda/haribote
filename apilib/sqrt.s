.globl	sqrt

# double sqrt(double x)
sqrt:
	fldl	4(%esp)		# x
	fsqrt
	ret
