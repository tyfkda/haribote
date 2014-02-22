TARGET=haribote.img

OBJDIR=obj
LIBDIR=lib

# Files included in the disk image.
DISK_FILES=\
	$(OBJDIR)/haribote.sys \
	haribote/ipl.s \
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
	$(OBJDIR)/type.hrb \
	$(OBJDIR)/bball.hrb \

APILIB=$(LIBDIR)/apilib.a

all:	$(TARGET)

.PHONY:	os lib apps

$(TARGET):	os lib apps
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

os:
	make -C haribote

lib:
	make -C apilib

apps:
	make -C apps

clean:
	rm -f $(OBJDIR)/*.o $(OBJDIR)/*.bin $(OBJDIR)/*.sys $(OBJDIR)/*.hrb $(OBJDIR)/*.map $(TARGET)
	rm -f $(APILIB)
