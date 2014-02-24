.globl	cos

# double cos(double x)
cos:
	fldl	4(%esp)		# x
	fcos
	ret
