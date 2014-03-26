.globl	io_hlt, io_cli, io_sti, io_stihlt
.globl	io_clts, io_fnsave, io_frstor
.globl	io_in8, io_in16, io_in32
.globl	io_out8, io_out16, io_out32
.globl	io_load_eflags, io_store_eflags
.globl	load_gdtr, load_idtr
.globl  load_cr0, store_cr0
.globl	load_tr
.globl	farjmp, farcall, start_app
.globl	asm_cons_putchar, asm_hrb_api, asm_end_app

.macro DEF_INTHANDLER	int_no
.globl	asm_inthandler\int_no
.extern	inthandler\int_no
asm_inthandler\int_no:
	push	%es
	push	%ds
	pushal
	mov	%esp, %eax
	push	%eax
	mov	%ss, %ax
	mov	%ax, %ds
	mov	%ax, %es
	call	inthandler\int_no
	pop	%eax
	popal
	pop	%ds
	pop	%es
.endm

.macro DEF_INTHANDLER_WITH_EXIT	int_no
.globl	asm_inthandler\int_no
.extern	inthandler\int_no
asm_inthandler\int_no:
	sti
	push	%es
	push	%ds
	pushal
	mov	%esp, %eax
	push	%eax
	mov	%ss, %ax
	mov	%ax, %ds
	mov	%ax, %es
	call	inthandler\int_no
	cmp	$0, %eax
	jne	asm_end_app
	pop	%eax
	popal
	pop	%ds
	pop	%es
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

# void io_clts(void)
io_clts:
	clts
	ret

# void fnsave(int* addr)
io_fnsave:
	mov	4(%esp), %eax	# addr
	fnsave	(%eax)
	ret

# void io_frstor(int* addr)
io_frstor:
	mov	4(%esp), %eax	# addr
	frstor	(%eax)
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

DEF_INTHANDLER 00
	iret

DEF_INTHANDLER 07
	iret

DEF_INTHANDLER_WITH_EXIT 0c
	add	$4, %esp	# Needed for int $0x0c
	iret

DEF_INTHANDLER_WITH_EXIT 0d
	add	$4, %esp	# Needed for int $0x0d
	iret

DEF_INTHANDLER 20
	iretl

DEF_INTHANDLER 21
	iretl

DEF_INTHANDLER 26
	iret

DEF_INTHANDLER 2c
	iretl

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
	mov	52(%esp), %ebp	# address of tss.esp0
	mov	%esp, (%ebp)	# Store esp for OS
	mov	%ss, 4(%ebp)	# Store ss for OS
	mov	%bx, %es
	mov	%bx, %ds
	mov	%bx, %fs
	mov	%bx, %gs
	# For retf
	or	$3, %ecx
	or	$3, %ebx
	push	%ebx		# ss for application
	push	%edx		# esp for application
	push	%ecx		# cs for application
	push	%eax		# eip for application
	retf

# API for hrb application, registered to int $0x40.
asm_hrb_api:
	sti
	push	%ds
	push	%es
	pushal			# Save registers
	pushal			# Push resisters as arguments for hrb_api
	mov	%ss, %ax
	mov	%ax, %ds	# Set os segment to ds and es, too
	mov	%ax, %es
	call	hrb_api
	cmp	$0, %eax
	jne	asm_end_app
	add	$32, %esp
	popal
	pop	%es
	pop	%ds
	iretl
asm_end_app:
	# eax is address of tss.esp0
	mov	(%eax), %esp
	popal
	ret			# Return to cmd_app
