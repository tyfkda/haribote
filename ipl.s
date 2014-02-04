.globl start
.code16

start:
	jmp	entry
	.byte	0x90
# Boot Parameter Block.
	.ascii "TEST IPL"	# Boot sector name (8 bytes)
	.word	512		# Size of sector (must be 512)
	.byte	1		# Size of cluster (must be 1)
	.word	1		# FAT start
	.byte	2		# FAT count
	.word	244		# Size of root dir
	.word	2880		# Size of this media (must be 2880)
	.byte	0xf0		# Media type (must be 0xf0)
	.word	9		# FAT length
	.word	18		# Number of sector in a track
	.word	2		# Number of head
	.long	0		# No partition
	.long	2880		# Drive size
	.byte	0,0,0x29	# Magic
	.long	0xffffffff	# Volume serial number
	.ascii	"HELLO-OS   "	# Disk name (11 bytes)
	.ascii	"FAT12   "	# Format name (8 bytes)
	.space	18,0

entry:
	movw	$0, %ax
	movw	%ax, %ss
	movw	$0x7c00, %sp
	movw    %ax, %ds
	movw	%ax, %es
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
	jmp		fin
	
msg:
	.ascii	"hello, world"
	.byte	0x0d, 0x0a, 0x00

.org 510
	.byte	0x55, 0xaa

# eof
