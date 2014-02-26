TARGET=haribote.img

SRCDIR=.
INC=-I ../include
OBJDIR=../obj
LIBDIR=../lib

SRCS=$(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/*.s)
OBJS=$(subst $(SRCDIR)/,$(OBJDIR)/,$(subst .s,.o,$(SRCS:%.c=%.o)))
APPS=$(OBJS:%.o=%.hrb)

APILIB=$(LIBDIR)/apilib.a

CFLAGS=-O2 --std=c99 -Wall -Wextra -Werror -nostdinc $(INC)
CFLAGS+=-fno-stack-protector  # Avoid reference for __stack_chk_fail

CRT0=$(LIBDIR)/crt0.o
LKSCRIPT=$(LIBDIR)/hrbapp.ls
LKOPT=-s
LKAPP=ld -T $(LKSCRIPT) $(LKOPT) $(CRT0)
APPLIBS=$(APILIB)

all:	$(APPS)

.SUFFIXES:
.SUFFIXS:	.o .c .s .hrb
.PRECIOUS:	$(OBJS)

$(OBJDIR)/%.o:	$(SRCDIR)/%.s
	as $< -o $@
$(OBJDIR)/%.o:	$(SRCDIR)/%.c
	gcc -c -o $@ $(CFLAGS) $<
$(OBJDIR)/%.hrb:	$(OBJDIR)/%.o $(APPLIBS) $(CRT0)
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/gview.o:	$(SRCDIR)/gview.c $(SRCDIR)/jpeg.ccc
$(OBJDIR)/gview.hrb:	$(OBJDIR)/gview.o bmp.obj $(APPLIBS) $(CRT0)
	$(LKAPP) -o $@ $< bmp.obj $(APPLIBS)
#$(OBJDIR)/bmp.o:	$(SRCDIR)/bmp.nasm
#	as $< -o $@

$(OBJDIR)/aobench.hrb:	$(OBJDIR)/aobench.o $(APPLIBS) $(CRT0)
	ld --defsym heap_size=4096K -T $(LKSCRIPT) $(LKOPT) $(CRT0) -o $@ $< $(APPLIBS)

$(APILIB):
	make -C ../apilib
