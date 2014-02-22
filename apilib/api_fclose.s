.globl	api_fclose
.include "syscall.def"

# void api_fclose(int fhandle)
api_fclose:
	mov	4(%esp), %eax	# fhandle
	syscall	API_FCLOSE
	ret
