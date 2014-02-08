.globl start
.code16

	.equ	CYLS, 10

start:
	jmp	entry
	.byte	0x90

	# Boot Parameter Block.
	.ascii "HARIBOTE"	# Boot sector name (8 bytes)
	.word	512		# Size of sector (must be 512)
	.byte	1		# Size of cluster (must be 1)
	.word	1		# FAT start
	.byte	2		# FAT count
	.word	224		# Size of root dir
	.word	2880		# Size of this media (must be 2880)
	.byte	0xf0		# Media type (must be 0xf0)
	.word	9		# FAT length
	.word	18		# Number of sector in a track
	.word	2		# Number of head
	.long	0		# No partition
	.long	0		# Drive size
	.byte	0,0,0x29	# Magic
	.long	0xffffffff	# Volume serial number
	.ascii	"HARIBOTE-OS"	# Disk name (11 bytes)
	.ascii	"FAT12   "	# Format name (8 bytes)
	.space	18,0

entry:
	movw	$0, %ax
	movw	%ax, %ss
	movw	$0x7c00, %sp
	movw    %ax, %ds

	# Read disk
	movw	$0x0820, %ax
	movw	%ax, %es
	movb	$0, %ch		# Cylinder 0
	movb	$0, %dh		# Head 0
	movb	$2, %cl		# Sector 2
readloop:
	mov	$0, %si		# Retry counter
retry:
	movb	$0x02, %ah	# Read disk
	movb	$1, %al		# 1 sector
	movw	$0, %bx
	movb	$0x00, %dl	# A drive
	int	$0x13		# Call BIOS
	jnc	next
	add	$1, %si
	cmp	$5, %si
	jae	error
	mov	$0x00, %ah
	mov	$0x00, %dl
	int	$0x13
	jmp	retry
next:
	mov	%es, %ax	# Forward address 0x200
	add	$0x0020, %ax
	mov	%ax, %es
	add	$1, %cl
	cmp	$18, %cl	# Read until 18 sector
	jbe	readloop
	mov	$1, %cl
	add	$1, %dh
	cmp	$2, %dh		# Read 2 heads
	jb	readloop
	mov	$0, %dh
	add	$1, %ch
	cmp	$CYLS, %ch
	jb	readloop	# Read until CYLS

	# Jump to loaded program
	mov	%ch, (0x0ff0)
	jmp	0xc200

error:
	mov	$0, %ax
	mov	%ax, %es
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

fin:
	hlt
	jmp	fin
	
msg:
	.ascii	"Disk read error"
	.byte	0x00

.org 510
	.byte	0x55, 0xaa

# eof
