Haribote OS
===========

Haribote OS is originally created by Hidemi Kawai,
introduced in a book called "[30日でできる! OS自作入門](http://amzn.to/1djVjZO)"

* x86, 32 bit OS
* Boot from floppy disk
* preemptive multi task
* Window like system
* 256 color screen mode

## Screenshots
![Screenshot](https://raw.github.com/tyfkda/haribote/master/screenshot/day30.png)

![fpu applications](https://raw.github.com/tyfkda/haribote/master/screenshot/fpu.png)

[See more](https://github.com/tyfkda/haribote/wiki/Screenshots-old)

## Required environment
* Linux
* gcc, as, ld
* make
* Ruby

Originally, Windows + nask + gcc and lots of tools are used for the original development environment.
But I changed to use Linux, and simplify required tools using Ruby.


## How to build
* Type `make` in the root directory
* `haribote.img` is created
* Burn image into floppy disk, or run disk image using PC emulator (e.g. VirtualBox)


## How to make your own .hrb application
### Create executable for Haribote OS
1. Create object files using gcc, or any
2. Use linker script [hrbapp.ls](https://github.com/tyfkda/haribote/blob/master/lib/hrbapp.ls) to make .hrb file
  * `$ ld -T hrbapp.ls --oformat binary -o <executable file name> <object files...>`

### Put executable onto floopy disk image
1. Prepare disk image
  1. Format: `$ tools/fat12img <image file name> format`
  2. Write boot sector: `$ tools/fat12img <image file name> write obj/ipl.bin 0`
  3. Write OS first: `$ tools/fat12img <image file name> save obj/haribote.sys`
2. Put executable file
  1. `$ tools/fat12img <image file name> save <.hrb file>`


## Memo
### Floppy disk
* 2HD : 1.4MB = 1,474,560 bytes (= 512 bytes x 18 sectors x 2 heads x 80 cylinders)
* 0x000000 - 0x0001ff : Boot sector
* 0x000200 - 0x0013ff : FAT
* 0x001400 - 0x0025ff : FAT (back up)
* 0x002600 - 0x0041ff : Root directory table
* 0x004200 - 0x167fff : Cluster data

### Haribote OS Memory layout
* 0x00007c00 - 0x00007dff : Boot sector is loaded (512 bytes)
* 0x00100000 - 0x00267fff : Floppy disk image (1440KB)
* 0x0026f800 - 0x0026ffff : IDT (2KB)
* 0x00270000 - 0x0027ffff : GDT (64KB)
* 0x00280000 - 0x002fffff : bootpack.hrb (512KB)
* 0x00300000 - 0x003fffff : Stack etc. (1MB)
