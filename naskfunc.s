.globl	io_hlt, io_cli, io_sti, io_stihlt
.globl	io_in8, io_in16, io_in32
.globl	io_out8, io_out16, io_out32
.globl	io_load_eflags, io_store_eflags
.globl	load_gdtr, load_idtr
.globl  load_cr0, store_cr0
.globl	load_tr
.globl	asm_inthandler20, asm_inthandler21, asm_inthandler2c
.globl	farjmp, farcall, start_app
.globl	asm_cons_putchar, asm_hrb_api
.extern inthandler20, inthandler21, inthandler2c

.macro asm_inthandler	c_inthandler
	push	%es
	push	%ds
	pushal
	mov	%ss, %ax
	cmp	$1 * 8, %ax
	jne	1f
	# Interrupt occurred while OS is running
	mov	%esp, %eax
	push	%ss		# Save ss
	push	%eax		# Save esp
	mov	%ss, %ax
	mov	%ax, %ds
	mov	%ax, %es
	call	\c_inthandler
	add	$8, %esp
	popal
	pop	%ds
	pop	%es
	iret
1:
	mov	$1 * 8, %eax
	mov	%ax, %ds	# Set ds for OS
	mov	(0x0fe4), %ecx
	add	$-8, %ecx
	mov	%ss, 4(%ecx)	# Save ss
	mov	%esp, (%ecx)	# Save esp
	mov	%ax, %ss
	mov	%ax, %es
	mov	%ecx, %esp
	call	\c_inthandler
	pop	%ecx
	pop	%eax
	mov	%ax, %ss	# Restore ss
	mov	%ecx, %esp	# Restore esp
	popal
	pop	%ds
	pop	%es
	iret
.endm

# void io_hlt(void)
io_hlt:
	hlt
	ret

# void io_cli(void)
io_cli:
	cli
	ret

# void io_sti(void)
io_sti:
	sti
	ret

# void io_stihlt(void)
io_stihlt:
	sti
	hlt
	ret

# int io_in8(int port)
io_in8:
	mov	4(%esp), %edx	# port
	mov	$0, %eax
	in	%dx, %al
	ret

# int io_in16(int port)
io_in16:
	mov	4(%esp), %edx	# port
	mov	$0, %eax
	in	%dx, %ax
	ret

# int io_in32(int port)
io_in32:
	mov	4(%esp), %edx	# port
	in	%dx, %eax
	ret

# int io_out8(int port, int data)
io_out8:
	mov	4(%esp), %edx	# port
	mov	8(%esp), %eax	# data
	out	%al, %dx
	ret

# int io_out16(int port, int data)
io_out16:
	mov	4(%esp), %edx	# port
	mov	8(%esp), %eax	# data
	out	%ax, %dx
	ret

# int io_out32(int port, int data)
io_out32:
	mov	4(%esp), %edx	# port
	mov	8(%esp), %eax	# data
	out	%eax, %dx
	ret

# int io_load_eflags(void)
io_load_eflags:
	pushfl			# push eflags
	pop	%eax
	ret

# void io_store_eflags(int eflags)
io_store_eflags:
	mov	4(%esp), %eax
	push	%eax
	popfl			# pop eflags
	ret

# void load_gdtr(int limit, int addr)
load_gdtr:
        mov     4(%esp), %ax
        mov     %ax, 6(%esp)
        lgdt    6(%esp)
        ret

# void load_idtr(int limit, int addr)
load_idtr:
        mov     4(%esp), %ax
        mov     %ax, 6(%esp)
        lidt    6(%esp)
        ret

# int load_cr0(void)
load_cr0:
        mov     %cr0, %eax
        ret

# int store_cr0(int cr0)
store_cr0:
        mov     4(%esp), %eax
        mov     %eax, %cr0
        ret

# void load_tr(int tr)
load_tr:
	LTR	4(%esp)		# tr
	ret

asm_inthandler20:
	asm_inthandler inthandler20

asm_inthandler21:
	asm_inthandler inthandler21

asm_inthandler2c:
	asm_inthandler inthandler2c

# void farjmp(int eip, int cs)
farjmp:
	ljmp	*4(%esp)	# eip, cs
	ret

# void farcall(int eip, int cs)
farcall:
	lcall	*4(%esp)	# eip, cs
	ret

# void start_app(int eip, int cs, int esp, int ds)
start_app:
	pushal
	mov	36(%esp), %eax	# eip
	mov	40(%esp), %ecx	# cs
	mov	44(%esp), %edx	# esp
	mov	48(%esp), %ebx	# ds/ss
	mov	%esp, (0x0fe4)	# Store esp for OS
	cli			# Disable interrupt while switching segment.
	mov	%bx, %es
	mov	%bx, %ss
	mov	%bx, %ds
	mov	%bx, %fs
	mov	%bx, %gs
	mov	%edx, %esp
	sti
	push	%ecx		# push cs and
	push	%eax		#   eip
	lcall	*(%esp)		# to call application
	#
	mov	$1 * 8, %eax	# ds/ss for OS
	cli
	mov	%ax, %es
	mov	%ax, %ss
	mov	%ax, %ds
	mov	%ax, %fs
	mov	%ax, %gs
	mov	(0x0fe4), %esp
	sti
	popal
	ret

asm_hrb_api:
	push	%ds
	push	%es
	pushal
	mov	$1 * 8, %eax
	mov	%ax, %ds	# Set ds for OS
	mov	(0x0fe4), %ecx	# esp for OS
	add	$-40, %ecx
	mov	%esp, 32(%ecx)	# Save esp for application
	mov	%ss, 36(%ecx)	# Save ss for application
	mov	(%esp), %edx
	mov	4(%esp), %ebx
	mov	%edx, (%ecx)
	mov	%ebx, 4(%ecx)
	mov	8(%esp), %edx
	mov	12(%esp), %ebx
	mov	%edx, 8(%ecx)
	mov	%ebx, 12(%ecx)
	mov	16(%esp), %edx
	mov	20(%esp), %ebx
	mov	%edx, 16(%ecx)
	mov	%ebx, 20(%ecx)
	mov	24(%esp), %edx
	mov	28(%esp), %ebx
	mov	%edx, 24(%ecx)
	mov	%ebx, 28(%ecx)

	mov	%ax, %es
	mov	%ax, %ss
	mov	%ecx, %esp
	sti

	call	hrb_api

	mov	32(%esp), %ecx	# Restore esp for application
	mov	36(%esp), %eax	# Restore ss for application
	cli
	mov	%ax, %ss
	mov	%ecx, %esp
	popal
	pop	%es
	pop	%ds
	iret
