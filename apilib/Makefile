
SRCDIR=.
INC=-I ../include
OBJDIR=../obj
LIBDIR=../lib

OBJS_API = \
	$(OBJDIR)/api_alloctimer.o \
	$(OBJDIR)/api_beep.o \
	$(OBJDIR)/api_boxfilwin.o \
	$(OBJDIR)/api_closewin.o \
	$(OBJDIR)/api_end.o \
	$(OBJDIR)/api_free.o \
	$(OBJDIR)/api_freetimer.o \
	$(OBJDIR)/api_getkey.o \
	$(OBJDIR)/api_initmalloc.o \
	$(OBJDIR)/api_inittimer.o \
	$(OBJDIR)/api_linewin.o \
	$(OBJDIR)/api_malloc.o \
	$(OBJDIR)/api_openwin.o \
	$(OBJDIR)/api_point.o \
	$(OBJDIR)/api_putchar.o \
	$(OBJDIR)/api_putstr0.o \
	$(OBJDIR)/api_putstr1.o \
	$(OBJDIR)/api_putstrwin.o \
	$(OBJDIR)/api_rand.o \
	$(OBJDIR)/api_refresh.o \
	$(OBJDIR)/api_settimer.o \
	$(OBJDIR)/api_sprintf.o \
	$(OBJDIR)/api_strcpy.o \

APILIB=$(LIBDIR)/apilib.a

CFLAGS=-O2 --std=c99 -Wall -Wextra -Werror $(INC)
CFLAGS+=-fno-stack-protector  # Avoid reference for __stack_chk_fail

all:	$(APILIB)

.SUFFIXS:	.c .s .o

$(OBJDIR)/%.o:	$(SRCDIR)/%.s
	as $< -o $@
$(OBJDIR)/%.o:	$(SRCDIR)/%.c
	gcc -c -o $@ $(CFLAGS) $<

$(APILIB):	$(OBJS_API)
	ar r $@ $^
