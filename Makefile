TARGET=boot.img

all:$(TARGET)

.SUFFIXS:	.s .o

.s.o:
	as $< -o $@

$(TARGET):	ipl.o ipl.ls
	ld -T ipl.ls ipl.o -o out.elf
	objcopy -O binary out.elf $@

clean:
	rm -f *.o $(TARGET) out.elf
