OUTPUT_FORMAT("binary");

SECTIONS {
    .head 0x0 : {
        LONG((ADDR(.bss) + SIZEOF(.bss) + 0xfff) & ~ 0xfff)      /*  0 : Size of stack+.data+heap (4KB align) */
        LONG(0x69726148)      /*  4 : Signature "Hari" */
        LONG(0)               /*  8 : Size of mmarea (4KB align) */
        LONG(ADDR(.data))     /* 12 : Stack address and .data destination address */
        LONG(SIZEOF(.data))   /* 16 : Size of .data */
        LONG(LOADADDR(.data)) /* 20 : Address of .data */
        LONG(0xE9000000)      /* 24 : 0xE9000000 (jump) */
        LONG(HariMain - 0x20) /* 28 : Entry address - 0x20 */
        LONG((ADDR(.bss) + SIZEOF(.bss) + 0xf) & ~ 0xf)       /* 32 : heap space (malloc) start address */
    }

    .text : {
        *(.text)
    }

    .data 0x2f0000 : AT(ADDR(.text) + SIZEOF(.text)) SUBALIGN(4) {
        *(.rodata*)
        *(.data)
    }

    .bss : AT(LOADADDR(.data) + SIZEOF(.data)) SUBALIGN(4) {
        *(.bss)
    }

    /DISCARD/ : { *(.eh_frame) }
}
