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

LKSCRIPT=$(LIBDIR)/hrbapp.ls
LKOPT=-s
LKAPP=ld -T $(LKSCRIPT) $(LKOPT)
APPLIBS=$(APILIB)

all:	$(APPS)

.SUFFIXES:
.SUFFIXS:	.o .c .s .hrb
.PRECIOUS:	$(OBJS)

$(OBJDIR)/%.o:	$(SRCDIR)/%.s
	as $< -o $@
$(OBJDIR)/%.o:	$(SRCDIR)/%.c
	gcc -c -o $@ $(CFLAGS) $<
$(OBJDIR)/%.hrb:	$(OBJDIR)/%.o $(APPLIBS)
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/gview.o:	$(SRCDIR)/gview.c $(SRCDIR)/jpeg.ccc
$(OBJDIR)/gview.hrb:	$(OBJDIR)/gview.o $(APPLIBS) bmp.obj
	$(LKAPP) -o $@ $^
#$(OBJDIR)/bmp.o:	$(SRCDIR)/bmp.nasm
#	as $< -o $@

$(OBJDIR)/aobench.hrb:	$(OBJDIR)/aobench.o $(APPLIBS)
	ld --defsym heap_size=4096K -T $(LKSCRIPT) $(LKOPT) -o $@ $^

$(APILIB):
	make -C ../apilib
