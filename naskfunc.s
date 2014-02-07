.globl	io_hlt, io_cli, io_sti, io_stihlt
.globl	io_in8, io_in16, io_in32
.globl	io_out8, io_out16, io_out32
.globl	io_load_eflags, io_store_eflags
.globl	load_gdtr, load_idtr
.globl	asm_inthandler21, asm_inthandler2c
.extern inthandler21, inthandler2c

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

asm_inthandler21:
        push    %es
        push    %ds
        pushal
        mov     %esp, %eax
        push    %eax
        mov     %ss, %ax
        mov     %ax, %ds
        mov     %ax, %es
        call    inthandler21
        pop     %eax
        popal
        pop     %ds
        pop     %es
        iretl

asm_inthandler2c:
        push    %es
        push    %ds
        pushal
        mov     %esp, %eax
        push    %eax
        mov     %ss, %ax
        mov     %ax, %ds
        mov     %ax, %es
        call    inthandler2c
        pop     %eax
        popal
        pop     %ds
        pop     %es
        iretl
