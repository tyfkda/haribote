TARGET=haribote.img

SRCDIR=.
INC=-I ../include
OBJDIR=../obj
LIBDIR=../lib

# Files included in the disk image.
APPS=\
	$(OBJDIR)/hello3.hrb \
	$(OBJDIR)/hello4.hrb \
	$(OBJDIR)/hello5.hrb \
	$(OBJDIR)/winhelo.hrb \
	$(OBJDIR)/stars.hrb \
	$(OBJDIR)/lines.hrb \
	$(OBJDIR)/walk.hrb \
	$(OBJDIR)/noodle.hrb \
	$(OBJDIR)/beepdown.hrb \
	$(OBJDIR)/color.hrb \
	$(OBJDIR)/color2.hrb \

APILIB=$(LIBDIR)/apilib.a

CFLAGS=-O2 --std=c99 -Wall -Wextra -Werror $(INC)
CFLAGS+=-fno-stack-protector  # Avoid reference for __stack_chk_fail

LKAPP=ld -T hrbapp.ls --oformat binary
APPLIBS=$(APILIB)

all:	$(APPS)

.SUFFIXS:	.c .s .o

$(OBJDIR)/%.o:	$(SRCDIR)/%.s
	as $< -o $@
$(OBJDIR)/%.o:	$(SRCDIR)/%.c
	gcc -c -o $@ $(CFLAGS) $<

$(OBJDIR)/hello3.hrb:	$(OBJDIR)/hello3.o $(APILIB)
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/hello4.hrb:	$(OBJDIR)/hello4.o $(APILIB)
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/hello5.hrb:	$(OBJDIR)/hello5.o $(APILIB)
	$(LKAPP) -o $@ $<

$(OBJDIR)/winhelo.hrb:	$(OBJDIR)/winhelo.o $(APILIB)
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/stars.hrb:	$(OBJDIR)/stars.o $(APILIB)
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/lines.hrb:	$(OBJDIR)/lines.o $(APILIB)
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/walk.hrb:	$(OBJDIR)/walk.o $(APILIB)
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/noodle.hrb:	$(OBJDIR)/noodle.o $(APILIB)
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/beepdown.hrb:	$(OBJDIR)/beepdown.o $(APILIB)
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/color.hrb:	$(OBJDIR)/color.o $(APILIB)
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/color2.hrb:	$(OBJDIR)/color2.o $(APILIB)
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/crack1.hrb:	$(OBJDIR)/crack1.o $(APILIB)
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/crack2.hrb:	$(OBJDIR)/crack2.o $(APILIB)
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/crack3.hrb:	$(OBJDIR)/crack3.o $(APILIB)
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/crack4.hrb:	$(OBJDIR)/crack4.o $(APILIB)
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/crack5.hrb:	$(OBJDIR)/crack5.o $(APILIB)
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/crack7.hrb:	$(OBJDIR)/crack7.o $(APILIB)
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/bug1.hrb:	$(OBJDIR)/bug1.o $(APILIB)
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/bug3.hrb:	$(OBJDIR)/bug3.o $(APILIB)
	$(LKAPP) -o $@ $< $(APPLIBS)
