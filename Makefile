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

SRCDIR=.
OBJDIR=obj

CFLAGS=-O2 --std=c99 -Wall -Wextra -Werror
CFLAGS+=-fno-stack-protector  # Avoid reference for __stack_chk_fail

LKAPP=ld -T hrbapp.ls --oformat binary
APPLIBS=$(OBJDIR)/a_nask.o

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

$(OBJDIR)/hello3.hrb:	$(OBJDIR)/hello3.o $(OBJDIR)/a_nask.o
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/hello4.hrb:	$(OBJDIR)/hello4.o $(OBJDIR)/a_nask.o
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/hello5.hrb:	$(OBJDIR)/hello5.o
	$(LKAPP) -o $@ $<

$(OBJDIR)/winhelo.hrb:	$(OBJDIR)/winhelo.o
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/stars.hrb:	$(OBJDIR)/stars.o
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/lines.hrb:	$(OBJDIR)/lines.o
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/walk.hrb:	$(OBJDIR)/walk.o
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/noodle.hrb:	$(OBJDIR)/noodle.o
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/beepdown.hrb:	$(OBJDIR)/beepdown.o
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/color.hrb:	$(OBJDIR)/color.o
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/color2.hrb:	$(OBJDIR)/color2.o
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/crack1.hrb:	$(OBJDIR)/crack1.o $(OBJDIR)/a_nask.o
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/crack2.hrb:	$(OBJDIR)/crack2.o $(OBJDIR)/a_nask.o
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/crack3.hrb:	$(OBJDIR)/crack3.o $(OBJDIR)/a_nask.o
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/crack4.hrb:	$(OBJDIR)/crack4.o $(OBJDIR)/a_nask.o
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/crack5.hrb:	$(OBJDIR)/crack5.o $(OBJDIR)/a_nask.o
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/bug1.hrb:	$(OBJDIR)/bug1.o $(OBJDIR)/a_nask.o
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/bug3.hrb:	$(OBJDIR)/bug3.o $(OBJDIR)/a_nask.o
	$(LKAPP) -o $@ $< $(APPLIBS)

clean:
	rm -f $(OBJDIR)/*.o $(OBJDIR)/*.bin $(OBJDIR)/*.sys $(OBJDIR)/*.hrb $(OBJDIR)/*.map $(TARGET)
