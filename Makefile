TARGET=haribote.iso

OBJDIR=obj
LIBDIR=lib
DATADIR=data

# Files included in the disk image.
DISK_FILES=\
	$(OBJDIR)/haribote.sys \
	$(OBJDIR)/type.hrb \
	$(OBJDIR)/del.hrb \
	$(OBJDIR)/echo.hrb \
	$(OBJDIR)/now.hrb \
	kernel/ipl.s \
	$(OBJDIR)/color2.hrb \
	$(OBJDIR)/bball.hrb \
	$(OBJDIR)/invader.hrb \
	$(OBJDIR)/tview.hrb \
	$(OBJDIR)/gview.hrb \
	$(DATADIR)/fujisan.jpg \
	$(DATADIR)/night.bmp \
	$(OBJDIR)/mandel.hrb \
	$(OBJDIR)/aobench.hrb \
	$(OBJDIR)/write.hrb \
	$(OBJDIR)/clock.hrb \
	$(OBJDIR)/lifegame.hrb \

APILIB=$(LIBDIR)/apilib.a

FAT12IMG=tools/fat12img

all:	$(TARGET)

.PHONY:	os lib apps

haribote.iso:	haribote.img
	mkisofs -b $< -o $@ -input-charset utf8 $<

haribote.img:	os lib apps
	$(FAT12IMG) $@ format
	$(FAT12IMG) $@ write obj/ipl.bin 0
	for filename in $(DISK_FILES); do \
	  $(FAT12IMG) $@ save $$filename; \
	done
os:
	make -C kernel

lib:
	make -C apilib

apps:
	make -C apps

clean:
	rm -f $(OBJDIR)/*.o $(OBJDIR)/*.bin $(OBJDIR)/*.sys $(OBJDIR)/*.hrb \
	  $(OBJDIR)/*.map $(LIBDIR)/*.a $(LIBDIR)/*.o \
	  haribote.img haribote.iso

font:
	ruby tools/makefont.rb < tools/biosfont.txt > kernel/fontdata.c

qemu:
	qemu-system-i386 -boot d -cdrom $(TARGET) -m 512

################################################
# Docker

docker_image ?= haribote
tag ?= dev

docker_args ?= -u $(shell id -u):$(shell id -g)\
	-v $(shell pwd):$(shell pwd) -w $(shell pwd) \
	-v /etc/localtime:/etc/localtime:ro \

docker-build:
	docker build docker/ -t $(docker_image):$(tag)

docker-make:
	@docker run -it --rm $(docker_args) $(docker_image):$(tag) make

docker-make-clean:
	@docker run -it --rm $(docker_args) $(docker_image):$(tag) make clean

docker-interactive:
	@docker run -it --rm $(docker_args) $(docker_image):$(tag)
