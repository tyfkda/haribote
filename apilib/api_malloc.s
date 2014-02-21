.globl	api_malloc

# void* api_malloc(int size)
api_malloc:
	push	%ebx
	mov	$9, %edx
	mov	%cs:(0x20), %ebx	# Address of memman for the task
	mov	8(%esp), %ecx		# Size
	int	$0x40
	pop	%ebx
	ret
