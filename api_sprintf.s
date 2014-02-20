.globl	sprintf

# int sprintf(char* buf, const char* format, ...)
sprintf:
	push	%ebx
	push	%ecx
	push	%edi
	mov	16(%esp), %ebx	# buf
	mov	20(%esp), %ecx	# format
	lea	24(%esp), %edi	# va_list
	mov	$10002, %edx
	int	$0x40
	pop	%edi
	pop	%ecx
	pop	%ebx
	ret
