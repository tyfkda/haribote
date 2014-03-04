.globl	api_malloc
.include "syscall.def"

# void* api_malloc(int size)
api_malloc:
	mov	%cs:(0x20), %ecx	# Address of memman for the task
	mov	4(%esp), %eax		# Size
	syscall	API_MALLOC
	ret
