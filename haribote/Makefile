
SRCDIR=.
INC=-I ../include
APILIBDIR=../apilib
OBJDIR=../obj
LIBDIR=../lib

OBJS= \
	$(OBJDIR)/bootpack.o \
	$(OBJDIR)/graphics.o \
	$(OBJDIR)/dsctbl.o \
	$(OBJDIR)/int.o \
	$(OBJDIR)/fifo.o \
	$(OBJDIR)/keyboard.o \
	$(OBJDIR)/mouse.o \
	$(OBJDIR)/memory.o \
	$(OBJDIR)/sheet.o \
	$(OBJDIR)/timer.o \
	$(OBJDIR)/mtask.o \
	$(OBJDIR)/window.o \
	$(OBJDIR)/console.o \
	$(OBJDIR)/file.o \
	$(OBJDIR)/naskfunc.o \
	$(OBJDIR)/fontdata.o \

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
	ld -N -e start -Ttext 0x7c00 -S --oformat binary -o $@ $<

$(OBJDIR)/haribote.sys:	$(OBJDIR)/asmhead.bin $(OBJDIR)/bootpack.bin
	cat $^ > $@

$(OBJDIR)/asmhead.bin:	$(OBJDIR)/asmhead.o
	ld -N -e start -Ttext 0xc200 -S --oformat binary -o $@ $<

$(OBJDIR)/bootpack.bin:	$(OBJS) $(APILIB)
	ld -Map $(OBJDIR)/bootpack.map -T harimain.ls --oformat binary -o $@ $^

$(APILIB):
	make -C ../apilib
