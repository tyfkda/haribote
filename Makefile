TARGET=haribote.img

OBJDIR=obj
LIBDIR=lib
DATADIR=data

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
	$(OBJDIR)/invader.hrb \
	$(OBJDIR)/calc.hrb \
	$(OBJDIR)/tview.hrb \
	$(OBJDIR)/gview.hrb \
	$(DATADIR)/fujisan.jpg \
	$(DATADIR)/night.bmp \

APILIB=$(LIBDIR)/apilib.a

FAT12IMG=tools/fat12img

all:	$(TARGET)

.PHONY:	os lib apps

$(TARGET):	os lib apps
	$(FAT12IMG) $@ format
	$(FAT12IMG) $@ write obj/ipl.bin 0
	for filename in $(DISK_FILES); do \
	  $(FAT12IMG) $@ save $$filename; \
	done
os:
	make -C haribote

lib:
	make -C apilib

apps:
	make -C apps

clean:
	rm -f $(OBJDIR)/*.o $(OBJDIR)/*.bin $(OBJDIR)/*.sys $(OBJDIR)/*.hrb $(OBJDIR)/*.map $(TARGET)
	rm -f $(APILIB)
