TARGET=fd0.img

CFLAGS=-O2 --std=c99 -Wall -Wextra -Werror
CFLAGS+=-fno-stack-protector  # Avoid reference for __stack_chk_fail

all:	$(TARGET)

.SUFFIXS:	.c .s .o

.s.o:
	as $< -o $@
.c.o:
	gcc -c -o $@ $(CFLAGS) $<

$(TARGET):	ipl.bin haribote.bin bootpack.bin
	cp ipl.bin $@
	ruby -e 'print "\0" * (0x4200-0x200)' >> $@
	cat haribote.bin >> $@
	cat bootpack.bin >> $@
	ruby -e 'size = File.size("$@"); print "\0" * (0x168000-size)' >> $@

ipl.bin:	ipl.o
	ld -N -e start -Ttext 0x7c00 -S --oformat binary -o $@ $<

haribote.bin:	haribote.o
	ld -N -e start -Ttext 0xc200 -S --oformat binary -o $@ $<

bootpack.bin:	bootpack.o graphic.o dsctbl.o stdio.o int.o fifo.o keyboard.o mouse.o naskfunc.o fontdata.o
	ld -T harimain.ls --oformat binary -o $@ $^

clean:
	rm -f *.o *.bin $(TARGET)
