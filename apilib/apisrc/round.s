.globl	round

# double round(double x)
round:
	fldl	4(%esp)	# x
	frndint			# Round floating point number
	ret
