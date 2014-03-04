.globl	api_delete
.include "syscall.def"

# int api_delete(const char* fname)
api_delete:
	mov	4(%esp), %eax	# fname
	syscall	API_DELETE
	ret
