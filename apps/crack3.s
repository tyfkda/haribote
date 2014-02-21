.globl	HariMain

HariMain:
	mov	$0x34, %al
	out	%al, $0x43	# io_out8(PIT_CTRL, 0x34)
	mov	$0xff, %al
	out	%al, $0x40	# io_out8(PIT_CNT0, 0xff)
	mov	$0xff, %al
	out	%al, $0x40	# io_out8(PIT_CNT0, 0xff)
	# Exit
	mov	$4, %edx
	int	$0x40
