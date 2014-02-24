.globl	sincos

# void sincos(double x, doube* p_sin, double* p_cos)
sincos:
	fldl	4(%esp)		# x
	fsincos
	mov	16(%esp), %eax
	fstpl	(%eax)		# *p_cos = cos(x)
	mov	12(%esp), %eax
	fstpl	(%eax)		# *p_sin = sin(x)
	ret
