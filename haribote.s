.globl start
.code16

start:
	# Put message
	movw	$msg, %si
putloop:
	movb	0(%si),	%al
	add	$1, %si
	cmpb	$0, %al
	je	fin
	movb	$0x0e, %ah      # put one char
	movw	$15, %bx        # color code
	int	$0x10           # call video bios
	jmp	putloop

error:
fin:
	hlt
	jmp		fin
	
msg:
	.ascii	"hello, world"
	.byte	0x0d, 0x0a, 0x00
