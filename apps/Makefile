TARGET=haribote.img

SRCDIR=.
INC=-I ../include
OBJDIR=../obj
LIBDIR=../lib

SRCS=$(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/*.cc) $(wildcard $(SRCDIR)/*.s)
OBJS=$(subst $(SRCDIR)/,$(OBJDIR)/,$(subst .s,.o,$(subst .cc,.o,$(SRCS:%.c=%.o))))
APPS=$(OBJS:%.o=%.hrb)

APILIB=$(LIBDIR)/apilib.a

CFLAGS=-O2 --std=c99 -Wall -Wextra -Werror -nostdinc $(INC)
CFLAGS+=-fno-stack-protector  # Avoid reference for __stack_chk_fail
CXXFLAGS=-O2 --std=c++11 -Wall -Wextra -Werror $(INC)
CXXFLAGS+=-fno-rtti -fno-exceptions
CXXFLAGS+=-fno-stack-protector  # Avoid reference for __stack_chk_fail

CRT0=$(LIBDIR)/crt0.o
LKSCRIPT=$(LIBDIR)/hrbapp.ls
LKOPT=-s
LKAPP=ld -T $(LKSCRIPT) $(LKOPT) $(CRT0)
APPLIBS=$(APILIB)

all:	$(APPS)

.SUFFIXES:
.SUFFIXS:	.o .c .cc .s .hrb
.PRECIOUS:	$(OBJS)

$(OBJDIR)/%.o:	$(SRCDIR)/%.s
	as $< -o $@
$(OBJDIR)/%.o:	$(SRCDIR)/%.c
	gcc -c -o $@ $(CFLAGS) $<
$(OBJDIR)/%.o:	$(SRCDIR)/%.cc
	g++ -c -o $@ $(CXXFLAGS) $<
$(OBJDIR)/%.hrb:	$(OBJDIR)/%.o $(APPLIBS) $(CRT0)
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/gview.o:	$(SRCDIR)/gview.c $(SRCDIR)/jpeg.ccc
$(OBJDIR)/gview.hrb:	$(OBJDIR)/gview.o bmp.obj $(APPLIBS) $(CRT0)
	$(LKAPP) -o $@ $< bmp.obj $(APPLIBS)
#$(OBJDIR)/bmp.o:	$(SRCDIR)/bmp.nasm
#	as $< -o $@

$(APILIB):
	make -C ../apilib
