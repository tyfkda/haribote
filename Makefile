TARGET=boot.img

all:$(TARGET)

.SUFFIXS:	.s .o

.s.o:
	as $< -o $@

$(TARGET):	ipl.bin haribote.bin ipl.ls
	cp ipl.bin $@
	ruby -e 'print "\0" * (0x4200-0x200)' >> $@
	cat haribote.bin >> $@
	ruby -e 'size = File.size("$@"); print "\0" * (0x168000-size)' >> $@

ipl.bin:	ipl.o
	ld -T ipl.ls --oformat binary -o $@ $<

haribote.bin:	haribote.o
	ld -N -e start -Ttext 0xc200 -S --oformat binary -o $@ $<

clean:
	rm -f *.o *.bin $(TARGET)
