.globl	tan
	
# double tan(double x)
tan:
	fldl	4(%esp)		# x
	fptan
	fdivr
	ret
