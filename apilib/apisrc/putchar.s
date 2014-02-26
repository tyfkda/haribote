.globl	putchar
.include "syscall.def"

# int putchar(int c)
putchar:
	mov	4(%esp), %eax	# c
	syscall	API_PUTCHAR
	ret
