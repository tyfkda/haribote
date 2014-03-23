
SRCDIR=apisrc
INC=-I ../include
OBJDIR=../obj
LIBDIR=../lib

SRCS=$(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/*.cc) $(wildcard $(SRCDIR)/*.s)
OBJS=$(subst $(SRCDIR)/,$(OBJDIR)/,$(subst .s,.o,$(subst .cc,.o,$(SRCS:%.c=%.o))))

APILIB=$(LIBDIR)/libhrb.a
CRT0=$(LIBDIR)/crt0.o

ASFLAGS=--32
CFLAGS=-m32 -O2 --std=c99 -Wall -Wextra -Werror -nostdinc $(INC)
CFLAGS+=-fno-stack-protector  # Avoid reference for __stack_chk_fail
CXXFLAGS=-m32 -O2 --std=c++11 -Wall -Wextra -Werror -nostdinc $(INC)

all:	$(APILIB) $(CRT0)

.SUFFIXS:	.o .c .cc .s

$(OBJDIR)/%.o:	$(SRCDIR)/%.s
	as -o $@ $(ASFLAGS) $<
$(OBJDIR)/%.o:	$(SRCDIR)/%.c
	gcc -c -o $@ $(CFLAGS) $<
$(OBJDIR)/%.o:	$(SRCDIR)/%.cc
	g++ -c -o $@ $(CXXFLAGS) $<

$(APILIB):	$(OBJS)
	ar r $@ $^

$(CRT0):	crt0/crt0.c
	gcc -c -o $@ $(CFLAGS) $<
