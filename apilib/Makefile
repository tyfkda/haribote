
SRCDIR=apisrc
INC=-I ../include
OBJDIR=../obj
LIBDIR=../lib

SRCS=$(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/*.cc) $(wildcard $(SRCDIR)/*.s)
OBJS=$(subst $(SRCDIR)/,$(OBJDIR)/,$(subst .s,.o,$(subst .cc,.o,$(SRCS:%.c=%.o))))

APILIB=$(LIBDIR)/apilib.a
CRT0=$(LIBDIR)/crt0.o

CFLAGS=-O2 --std=c99 -Wall -Wextra -Werror $(INC)
CFLAGS+=-fno-stack-protector  # Avoid reference for __stack_chk_fail
CXXFLAGS=-O2 --std=c++11 -Wall -Wextra -Werror $(INC)

all:	$(APILIB) $(CRT0)

.SUFFIXS:	.o .c .cc .s

$(OBJDIR)/%.o:	$(SRCDIR)/%.s
	as $< -o $@
$(OBJDIR)/%.o:	$(SRCDIR)/%.c
	gcc -c -o $@ $(CFLAGS) $<
$(OBJDIR)/%.o:	$(SRCDIR)/%.cc
	g++ -c -o $@ $(CXXFLAGS) $<

$(APILIB):	$(OBJS)
	ar r $@ $^

$(CRT0):	crt0/crt0.c
	gcc -c -o $@ $(CFLAGS) $<
