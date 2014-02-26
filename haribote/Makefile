
SRCDIR=.
INC=-I ../include
APILIBDIR=../apilib
OBJDIR=../obj
LIBDIR=../lib

SRCS=$(wildcard $(SRCDIR)/*.c)
SRCS+=$(SRCDIR)/asmhead.s $(SRCDIR)/naskfunc.s
OBJS=$(subst $(SRCDIR)/,$(OBJDIR)/,$(subst .s,.o,$(SRCS:%.c=%.o)))

CFLAGS=-O2 --std=c99 -Wall -Wextra -Werror $(INC)
CFLAGS+=-fno-stack-protector  # Avoid reference for __stack_chk_fail

APILIB=$(LIBDIR)/apilib.a

all:	$(OBJDIR)/ipl.bin $(OBJDIR)/haribote.sys

.SUFFIXS:	.c .s .o

$(OBJDIR)/%.o:	$(SRCDIR)/%.s
	as $< -o $@
$(OBJDIR)/%.o:	$(SRCDIR)/%.c
	gcc -c -o $@ $(CFLAGS) $<
$(OBJDIR)/%.o:	$(APILIBDIR)/%.c
	gcc -c -o $@ $(CFLAGS) $<

$(OBJDIR)/ipl.bin:	$(OBJDIR)/ipl.o
	ld -N -e start -Ttext 0x7c00 -s --oformat binary -o $@ $<

$(OBJDIR)/haribote.sys:	$(OBJDIR)/asmhead.bin $(OBJDIR)/bootpack.bin
	cat $^ > $@

$(OBJDIR)/asmhead.bin:	$(OBJDIR)/asmhead.o
	ld -N -e start -Ttext 0xc200 -s --oformat binary -o $@ $<

$(OBJDIR)/bootpack.bin:	$(OBJS) $(APILIB)
	ld -Map $(OBJDIR)/bootpack.map -T harimain.ls -s -o $@ $^

$(APILIB):
	make -C ../apilib
