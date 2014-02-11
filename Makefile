TARGET=haribote.img

SRCDIR=.
OBJDIR=obj

CFLAGS=-O2 --std=c99 -Wall -Wextra -Werror
CFLAGS+=-fno-stack-protector  # Avoid reference for __stack_chk_fail

all:	$(TARGET)

$(TARGET):	$(OBJDIR)/ipl.bin $(OBJDIR)/haribote.sys
	cp $(OBJDIR)/ipl.bin $@
	ruby -e 'print "\0" * (0x4200-0x200)' >> $@
	cat $(OBJDIR)/haribote.sys >> $@
	ruby -e 'size = File.size("$@"); print "\0" * (0x168000-size)' >> $@

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

$(OBJDIR)/bootpack.bin:	$(OBJDIR)/bootpack.o $(OBJDIR)/graphics.o $(OBJDIR)/dsctbl.o $(OBJDIR)/stdio.o $(OBJDIR)/int.o $(OBJDIR)/fifo.o $(OBJDIR)/keyboard.o $(OBJDIR)/mouse.o $(OBJDIR)/memory.o $(OBJDIR)/sheet.o $(OBJDIR)/timer.o $(OBJDIR)/mtask.o $(OBJDIR)/naskfunc.o $(OBJDIR)/fontdata.o
	ld -Map $(OBJDIR)/bootpack.map -T harimain.ls --oformat binary -o $@ $^

clean:
	rm -f $(OBJDIR)/*.o $(OBJDIR)/*.bin $(OBJDIR)/*.sys $(OBJDIR)/*.map $(TARGET)
