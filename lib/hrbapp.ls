OUTPUT_FORMAT("binary");

_stack_size = 64K;
_heap_size = 1024K;

MEMORY {
  rom (rx) : ORIGIN = 0, LENGTH = 1024K
  ram (rwx) : ORIGIN = 64K, LENGTH = 1024K
}

SECTIONS {
    .head : {
        LONG((_stack_size + SIZEOF(.data) + SIZEOF(.bss) + _heap_size + 0xfff) & ~ 0xfff)      /*  0 : Size of data segment (4KB align) */
        LONG(0x69726148)      /*  4 : Signature "Hari" */
        LONG(0)               /*  8 : Size of mmarea (4KB align) */
        LONG(_stack_size)     /* 12 : Stack address and .data destination address */
        LONG(SIZEOF(.data))   /* 16 : Size of .data */
        LONG(LOADADDR(.data)) /* 20 : Address of .data */
        LONG(0xE9000000)      /* 24 : 0xE9000000 (jump) */
        LONG(HariMain - 0x20) /* 28 : Entry address - 0x20 */
        LONG((ADDR(.bss) + SIZEOF(.bss) + 0xf) & ~ 0xf)       /* 32 : heap space (malloc) start address */
    } > rom

    .text : {
        *(.text)
    } > rom

    .data : {
        *(.data)
        *(.rodata*)
        . = ALIGN(16);
    } > ram AT > rom

    .bss : AT(LOADADDR(.data) + SIZEOF(.data)) SUBALIGN(4) {
        *(.bss)
        . = ALIGN(16);
    } > ram

    /DISCARD/ : { *(.eh_frame) }
}
