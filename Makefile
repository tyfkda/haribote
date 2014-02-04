TARGET=boot.img

all:$(TARGET)

.SUFFIXS:	.s .o

.s.o:
	as $< -o $@

$(TARGET):	ipl.o ipl.ls
	ld -T ipl.ls --oformat binary -o $@ ipl.o

clean:
	rm -f *.o *.bin $(TARGET)
