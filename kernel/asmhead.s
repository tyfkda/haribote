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

	.equ	VBEMODE, 0x105
	# 0x100 :  640 x  400 x 8bit color
	# 0x101 :  640 x  480 x 8bit color
	# 0x103 :  800 x  600 x 8bit color
	# 0x105 : 1024 x  768 x 8bit color
	# 0x107 : 1280 x 1024 x 8bit color
	# 0x111 :  640 x  480 x 16bit color
	# 0x112 :  640 x  480 x 24bit color

start:
	mov	$0x9000, %ax
	mov	%ax, %es
	mov	$0, %di
	mov	$0x4f00, %ax
	int	$0x10
	cmp	$0x004f, %ax
	jne	scrn320

	# Check VBE version.
	mov	%es:4(%di), %ax
	cmp	$0x0200, %ax
	jb	scrn320

	# Get screen mode information.
	mov	$VBEMODE, %cx
	mov	$0x4f01, %ax
	int	$0x10
	cmp	$0x004f, %ax
	jne	scrn320

	# Check screen mode information.
	#cmpb	$8, %es:0x19(%di)
	#jne	scrn320
	#cmpb	$4, %es:0x1b(%di)
	#jne	scrn320
	mov	%es:0(%di), %ax
	and	$0x0080, %ax
	jz	scrn320

	# Switch screen mode.
	mov	$VBEMODE+0x4000, %bx
	mov	$0x4f02, %ax
	int	$0x10

	mov	%es:0x19(%di), %al
	mov	%al, (VMODE)
	mov	%es:0x12(%di), %ax
	mov	%ax, (SCRNX)
	mov	%es:0x14(%di), %ax
	mov	%ax, (SCRNY)
	mov	%es:0x28(%di), %eax
	mov	%eax, (VRAM)
	jmp	keystatus

scrn320:
	mov	$0x13, %al	# VGA 320x200x8bit color
	mov	$0x00, %ah
	int	$0x10
	movb	$8, (VMODE)
	movw	$320, (SCRNX)
	movw	$200, (SCRNY)
	movl	$0x000a0000, (VRAM)

	# Get keyboard LED state from BIOS
keystatus:
	mov	$0x02, %ah
	int	$0x16
	movb	%al, (LEDS)

	# Prevent interrupt for PIC
	mov	$0xff, %al
	out	%al, $0x21
	nop			# Avoid using `out` intstruction sequentially.
	out	%al, $0xa1

	cli			# Prevent interrupt for CPU level

	# Set key repeat speed.
	call	waitkbdout
	mov	$0xf3, %al	# Typematic rate/delay setting.
	out	%al, $0x60
	call	waitkbdout
	mov	$0x00, %al	# bit0-4: Typematic rate (30 chars/sec), bit5-6: Typematic delay(250 ms)
	out	%al, $0x60

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
	and	$0x7fffffff, %eax	# Disable paging
	or	$0x00000001, %eax	# Go to protect mode
	mov	%eax, %CR0
	jmp	pipelineflash
pipelineflash:
	mov	$1*8, %ax
	mov	%ax, %ds
	mov	%ax, %es
	mov	%ax, %fs
	mov	%ax, %gs
	mov	%ax, %ss

	# Copy disk data
	mov	$0x7c00, %esi
	mov	$DSKCAC, %edi
	mov	$512/4, %ecx
	call	memcpy4

	mov	$DSKCAC0+512, %esi	# source
	mov	$DSKCAC+512, %edi	# destination
	mov	$0, %ecx
	movb	(CYLS), %cl
	imul	$512*18*2/4, %ecx
	sub	$512/4, %ecx
	call	memcpy4

	# Copy OS code to BOTPAK
	mov	$bootpack, %esi	# source
	mov	$BOTPAK, %edi	# destination
	mov	$512*1024/4, %ecx
	call	memcpy4

	# Move OS data into appropriate address.
	mov	$BOTPAK, %ebx
	mov	16(%ebx), %ecx
	add	$3, %ecx	# ECX += 3
	shr	$2, %ecx	# ECX /= 4
	mov	20(%ebx), %esi	# source
	add	%ebx, %esi
	mov	12(%ebx), %edi	# destination
	call	memcpy4

	# Boot OS
	mov	12(%ebx), %esp
	ljmp	$2*8, $0x0000001b

waitkbdout:
	in	$0x64, %al
	and	$0x02, %al
	in	$0x60, %al	# Read for dummy
	jnz	waitkbdout
	ret

memcpy4:
	mov	(%esi), %eax
	add	$4, %esi
	mov	%eax, (%edi)
	add	$4, %edi
	sub	$1, %ecx
	jnz	memcpy4
	ret

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
