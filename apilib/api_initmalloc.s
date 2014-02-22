.globl	api_initmalloc
.include "syscall.def"

# void api_initmalloc(void)
api_initmalloc:
	push	%ebx
	mov	%cs:(0x20), %ebx	# Address of malloc start
	mov	%ebx, %eax
	add	$32 * 1024, %eax	# Add 32KB
	mov	%cs:(0x00), %ecx	# Size of data segment
	sub	%eax, %ecx
	syscall	API_INITMALLOC
	pop	%ebx
	ret
