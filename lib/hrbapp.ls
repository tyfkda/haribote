OUTPUT_FORMAT("binary");

/*
 * You can overwrite these settings using '--defsym' command line option:
 *   $ ld --defsym stack_size=256K --defsym heap_size=1024K -T hrbapp.ls ...
 */
stack_size = DEFINED(stack_size) ? stack_size : 64K;
heap_size = DEFINED(heap_size) ? heap_size : 1024K;


MEMORY {
  rom (rx) : ORIGIN = 0, LENGTH = 1024K
  ram (rwx) : ORIGIN = 64K, LENGTH = 1024K
}

SECTIONS {
    .head : {
        LONG((stack_size + SIZEOF(.data) + SIZEOF(.bss) + heap_size + 0xfff) & ~ 0xfff)      /*  0 : Size of data segment (4KB align) */
        LONG(0x69726148)      /*  4 : Signature "Hari" */
        LONG(0)               /*  8 : Size of mmarea (4KB align) */
        LONG(stack_size)      /* 12 : Stack address and .data destination address */
        LONG(SIZEOF(.data))   /* 16 : Size of .data */
        LONG(ADDR(.data))     /* 20 : Address of .data */
        LONG(0xE9000000)      /* 24 : 0xE9000000 (jump) */
        LONG(HariMain - 0x20) /* 28 : Entry address - 0x20 */
        LONG((ADDR(.bss) + SIZEOF(.bss) + 0xf) & ~ 0xf)       /* 32 : heap space (malloc) start address */
    } > rom

    .text : {
        *(.text.startup.*)
        *(.text*)
    } > rom

    .data : {
        *(.data*)
        *(.rodata*)

        . = ALIGN(4);
        __preinit_array_start = .;
        *(.preinit_array)
        __preinit_array_end = .;

        __init_array_start = .;
        *(SORT(.init_array.*))
        *(.init_array)
        __init_array_end = .;

        __fini_array_start = .;
        *(SORT(.fini_array.*))
        *(.fini_array)
        __fini_array_end = .;
        . = ALIGN(16);
    } > ram AT > rom

    .bss : AT(LOADADDR(.data) + SIZEOF(.data)) SUBALIGN(4) {
        *(.bss)
        . = ALIGN(16);
    } > ram

    /DISCARD/ : { *(.eh_frame) }
}
