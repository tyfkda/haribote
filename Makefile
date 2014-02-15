TARGET=haribote.img

# Files included in the disk image.
DISK_FILES=\
	$(OBJDIR)/haribote.sys \
	$(OBJDIR)/crack1.hrb \
	$(OBJDIR)/crack2.hrb \
	$(OBJDIR)/crack3.hrb \
	$(OBJDIR)/crack4.hrb \
	$(OBJDIR)/crack5.hrb \
	$(OBJDIR)/bug1.hrb \
	$(OBJDIR)/bug3.hrb \
	ipl.s \
	$(OBJDIR)/hello.hrb \
	$(OBJDIR)/hello3.hrb \
	$(OBJDIR)/hello4.hrb \

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

$(OBJDIR)/hello.hrb:	$(OBJDIR)/hello.o
	ld -N -e start -Ttext 0 -S --oformat binary -o $@ $<

$(OBJDIR)/hello3.hrb:	$(OBJDIR)/hello3.o $(OBJDIR)/a_nask.o
	$(LKAPP) -o $@ $< $(APPLIBS)

$(OBJDIR)/hello4.hrb:	$(OBJDIR)/hello4.o $(OBJDIR)/a_nask.o
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
