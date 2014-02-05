.globl start
.code16

	#.org	0xc200

	.equ	BOTPAK, 0x00280000	# Load address for bootpack
	.equ	DSKCAC, 0x00100000	# Address for disk cache
	.equ	DSKCAC0, 0x00008000	# Address for disk cache (real mode)

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

	# Prevent interrupt for PIC
	mov	$0xff, %al
	out	%al, $0x21
	nop			# Avoid using out intstruction .
	out	%al, $0xa1

	cli			# Prevent interrupt for CPU level

	# Set A20GATE to use more than 1MB from CPU
	call	waitkbdout
	mov	$0xd1, %al
	out	%al, $0x64
	call	waitkbdout
	mov	$0xdf, %al
	out	%al, $0x60
	call	waitkbdout

	# Transit to protect mode
	lgdt	(GDTR0)
	mov	%CR0, %eax
	and	$0x7fffffff, %eax
	or	$0x00000001, %eax
	mov	%eax, %CR0
	jmp	pipelineflash
pipelineflash:
	mov	$1*8, %ax
	mov	%ax, %ds
	mov	%ax, %es
	mov	%ax, %fs
	mov	%ax, %gs
	mov	%ax, %ss

	# Copy bootpack
	mov	$bootpack, %esi	# source
	mov	$BOTPAK, %edi	# destination
	mov	$512*1024/4, %ecx
	call	memcpy

	# Copy disk data
	mov	$0x7c00, %esi
	mov	$DSKCAC, %edi
	mov	$512/4, %ecx
	call	memcpy

	mov	$DSKCAC0+512, %esi	# source
	mov	$DSKCAC+512, %edi	# destination
	mov	$0, %ecx
	movb	(CYLS), %cl
	imul	$512*18*2/4, %ecx
	sub	$512/4, %ecx
	call	memcpy

	# boot bootpack
	mov	$BOTPAK, %ebx
	mov	16(%ebx), %ecx
	add	$3, %ecx	# ECX += 3
	shr	$2, %ecx	# ECX /= 4
	jz	skip
	mov	20(%ebx), %esi	# source
	add	%ebx, %esi
	mov	12(%ebx), %edi	# destination
	call	memcpy
skip:
	mov	12(%ebx), %esp
	ljmp	$2*8, $0x0000001b

waitkbdout:
	in	$0x64, %al
	and	$0x02, %al
	in	$0x60, %al	# Read for dummy
	jnz	waitkbdout
	ret

memcpy:
	mov	(%esi), %eax
	add	$4, %esi
	mov	%eax, (%edi)
	add	$4, %edi
	sub	$1, %ecx
	jnz	memcpy
	ret
	
msg:
	.ascii	"hello, world"
	.byte	0x0d, 0x0a, 0x00

	.align	16
GDT0:
	.space	8,0				# Null selector
	.word	0xffff,0x0000,0x9200,0x00cf	# Read/write able segment 32bit
	.word	0xffff,0x0000,0x9a28,0x0047	# Executable segment 32bit (for bootpack): 0x00280000

	.word	0
GDTR0:
	.word	8*3-1
	.long	GDT0

bootpack:
