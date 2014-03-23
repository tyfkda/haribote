.globl	log

# double log(double x)
log:
	#fldl2e
	#fld1
	#fdivp			# 1.0 / log2(e)
	fldl	.inv_log2e	# 1.0 / log2(e)
	fldl	4(%esp)		# x
	fyl2x			# log2(x) / log2(e) = log_e(x)
	ret

	.section	.rodata
.inv_log2e:			# 1.0 / log_2(e) ~= 0.69314718055995
	.long	4277811737	# 0xFEFA3A19
	.long	1072049730	# 0x3FE62E42
