.globl	HariMain

HariMain:
	call	$2*8, $0x41d2	# call io_cli directly
	mov	$4, %edx
	int	$0x40
