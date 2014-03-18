.globl	atan
	
# double atan(double x)
atan:
	fldl	4(%esp)		# x
	fld1
	fpatan
	ret
