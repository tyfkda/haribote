.globl	api_malloc
.include "syscall.def"

# void* api_malloc(int size)
api_malloc:
	push	%ebx
	mov	%cs:(0x20), %ebx	# Address of memman for the task
	mov	8(%esp), %ecx		# Size
	syscall	API_MALLOC
	pop	%ebx
	ret
