.globl	lround

# long round(double x)
lround:
	fldl	4(%esp)	# x
	frndint			# Round floating point number
	fistpl	4(%esp)		# Convert double to long
	movl	4(%esp), %eax	# Move it to int register
	ret
