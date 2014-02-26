.globl	api_delete
.include "syscall.def"

# int api_delete(const char* fname)
api_delete:
	push	%ebx
	mov	8(%esp), %ebx	# fname
	syscall	API_DELETE
	pop	%ebx
	ret
