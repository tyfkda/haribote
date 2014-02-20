TARGET=haribote.img

# Files included in the disk image.
DISK_FILES=\
	$(OBJDIR)/haribote.sys \
	ipl.s \
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

SRCDIR=.
OBJDIR=obj
LIBDIR=lib

APILIB=$(LIBDIR)/apilib.a

CFLAGS=-O2 --std=c99 -Wall -Wextra -Werror
CFLAGS+=-fno-stack-protector  # Avoid reference for __stack_chk_fail

LKAPP=ld -T hrbapp.ls --oformat binary
APPLIBS=$(APILIB)

all:	$(TARGET)

$(TARGET):	$(OBJDIR)/ipl.bin $(OBJDIR)/haribote.sys $(DISK_FILES)
	cat $(OBJDIR)/ipl.bin > $@
	# FAT x 2
	ruby tools/fat.rb $(DISK_FILES) >> $@
	ruby tools/padding.rb -cps 0x1400 $@ >> $@
	ruby tools/fat.rb $(DISK_FILES) >> $@
	# Disk root directory.
	ruby tools/padding.rb -cps 0x2600 $@ >> $@
	ruby tools/fileinfo.rb $(DISK_FILES) >> $@
	# Put file entities, aligned with 512 bytes.
	ruby tools/padding.rb -cps 0x4200 $@ >> $@
	for filename in $(DISK_FILES); do \
	  cat $$filename >> $@; \
	  ruby tools/padding.rb -ps 512 $$filename >> $@; \
	done
	ruby tools/padding.rb -cps 0x168000 $@ >> $@

.SUFFIXS:	.c .s .o

$(OBJDIR)/%.o:	$(SRCDIR)/%.s
	as $< -o $@
$(OBJDIR)/%.o:	$(SRCDIR)/%.c
	gcc -c -o $@ $(CFLAGS) $<

$(OBJDIR)/ipl.bin:	$(OBJDIR)/ipl.o
	ld -N -e start -Ttext 0x7c00 -S --oformat binary -o $@ $<

$(OBJDIR)/haribote.sys:	$(OBJDIR)/asmhead.bin $(OBJDIR)/bootpack.bin
	cat $^ > $@

$(OBJDIR)/asmhead.bin:	$(OBJDIR)/asmhead.o
	ld -N -e start -Ttext 0xc200 -S --oformat binary -o $@ $<

$(OBJDIR)/bootpack.bin:	$(OBJDIR)/bootpack.o $(OBJDIR)/graphics.o $(OBJDIR)/dsctbl.o $(OBJDIR)/stdio.o $(OBJDIR)/int.o $(OBJDIR)/fifo.o $(OBJDIR)/keyboard.o $(OBJDIR)/mouse.o $(OBJDIR)/memory.o $(OBJDIR)/sheet.o $(OBJDIR)/timer.o $(OBJDIR)/mtask.o $(OBJDIR)/window.o $(OBJDIR)/console.o $(OBJDIR)/file.o $(OBJDIR)/naskfunc.o $(OBJDIR)/fontdata.o
	ld -Map $(OBJDIR)/bootpack.map -T harimain.ls --oformat binary -o $@ $^

$(APILIB):	$(OBJS_API)
	ar r $@ $^

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

clean:
	rm -f $(OBJDIR)/*.o $(OBJDIR)/*.bin $(OBJDIR)/*.sys $(OBJDIR)/*.hrb $(OBJDIR)/*.map $(TARGET)
	rm -f $(APILIB)
