
SRCDIR=.
INC=-I ../include
APILIBDIR=../apilib
OBJDIR=../obj
LIBDIR=../lib

SRCS=$(wildcard $(SRCDIR)/*.c)
SRCS+=$(SRCDIR)/naskfunc.s
OBJS=$(subst $(SRCDIR)/,$(OBJDIR)/,$(subst .s,.o,$(SRCS:%.c=%.o)))

ASFLAGS=--32
CFLAGS=-m32 -O2 --std=c99 -Wall -Wextra -Werror -nostdinc $(INC)
CFLAGS+=-fno-stack-protector  # Avoid reference for __stack_chk_fail
CFLAGS+=-fno-pie -Wno-error=shift-negative-value

APILIB=$(LIBDIR)/libhrb.a

all:	$(OBJDIR)/ipl.bin $(OBJDIR)/haribote.sys

.SUFFIXS:	.c .s .o

$(OBJDIR)/%.o:	$(SRCDIR)/%.s
	as -o $@ $(ASFLAGS) $<
$(OBJDIR)/%.o:	$(SRCDIR)/%.c
	gcc -c -o $@ $(CFLAGS) $<
$(OBJDIR)/%.o:	$(APILIBDIR)/%.c
	gcc -c -o $@ $(CFLAGS) $<

$(OBJDIR)/ipl.bin:	$(OBJDIR)/ipl.o
	ld -m elf_i386 -N -e start -Ttext 0x7c00 -s --oformat binary -o $@ $<

$(OBJDIR)/haribote.sys:	$(OBJDIR)/asmhead.bin $(OBJDIR)/bootpack.bin
	cat $^ > $@

$(OBJDIR)/asmhead.bin:	$(OBJDIR)/asmhead.o
	ld -m elf_i386 -N -e start -Ttext 0xc200 -s --oformat binary -o $@ $<

$(OBJDIR)/bootpack.bin:	$(OBJS) $(APILIB)
	ld -m elf_i386 -Map $(OBJDIR)/bootpack.map -T harimain.ls -s -o $@ $(OBJS) $(APILIB)

$(APILIB):
	make -C ../apilib
