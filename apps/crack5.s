.globl	main

main:
	call	$2*8, $0x41d2	# call io_cli directly
	mov	$0, %eax
	ret
