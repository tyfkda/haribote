.globl start
.code16

	#.org	0xc200

	.equ	CYLS, 0x0ff0	# Set by boot sector
	.equ	LEDS, 0x0ff1
	.equ	VMODE, 0x0ff2	# Depth of colors
	.equ	SCRNX, 0x0ff4	# Resolution x (screen x)
	.equ	SCRNY, 0x0ff6	# Resolution y (screen y)
	.equ	VRAM, 0x0ff8	# Graphics buffer

start:
	mov	$0x13, %al	# VGA graphics, 320x200x8bit color
	mov	$0x00, %ah
	int	$0x10

	movb	$8, (VMODE)
	movw	$320, (SCRNX)
	movw	$200, (SCRNY)
	movl	$0x000a0000, (VRAM)

	# Get keyboard LED state from BIOS
	mov	$0x02, %ah
	int	$0x16
	movb	%al, (LEDS)

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
