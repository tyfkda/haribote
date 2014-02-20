.globl	api_free

# void api_free(void* addr, int size)
api_free:
	push	%ebx
	mov	$10, %edx
	mov	%cs:(0x20), %ebx	# Address of memman for the task
	mov	8(%esp), %ecx		# addr
	mov	12(%esp), %ecx		# Size
	int	$0x40
	pop	%ebx
	ret
