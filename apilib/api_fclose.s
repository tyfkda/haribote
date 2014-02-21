.globl	api_fclose

# void api_fclose(int fhandle)
api_fclose:
	mov	$22, %edx
	mov	4(%esp), %eax	# fhandle
	int	$0x40
	ret
