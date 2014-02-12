.globl	start

start:
	mov	$'H', %al
	int	$0x40
	mov	$'e', %al
	int	$0x40
	mov	$'l', %al
	int	$0x40
	mov	$'l', %al
	int	$0x40
	mov	$'o', %al
	int	$0x40
	mov	$'\n', %al
	int	$0x40
	retf
