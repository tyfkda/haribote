Haribote OS
===========

Haribote OS is originally created by Hidemi Kawai,
introduced in a book called "[OS自作入門](http://amzn.to/1djVjZO)"

* x86, 32 bit OS
* Boot from floppy disk



= Required environment
* Linux + gcc
* Makefile
* ruby (for tools)


= How to build
* Type `make` in the root directory
* `haribote.img` is created
* Burn image into floppy disk, or run disk image using PC emulator (e.g. VirtualBox)


= Memo
== Disk layout
* 2HD : 1.4MB = 1,474,560 bytes (512 bytes x 18 sectors x 2 heads x 80 cylinders)
* 0x000000 - 0x0001ff : Boot sector
* 0x000200 - 0x0013ff : FAT
* 0x001400 - 0x0025ff : FAT (back up)
* 0x002600 - 0x0041ff : Root directory table
* 0x004200 - 0x168000 : Cluster data

== Memory layout
* 0x00007c00 - 0x00007dff : Boot sector is loaded (512 bytes)
* 0x00100000 - 0x00267fff : Floppy disk image (1440KB)
* 0x0026f800 - 0x0026ffff : IDT (2KB)
* 0x00270000 - 0x0027ffff : GDT (64KB)
* 0x00280000 - 0x002fffff : bootpack.hrb (512KB)
* 0x00300000 - 0x003fffff : Stack etc. (1MB)
