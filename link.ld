ENTRY(loader)

SECTIONS {
    /* VMA: Diz para o GCC que o código VAI rodar no endereço Virtual de 3GB + 1MB */
    . = 0xC0100000;

    /* LMA: Mas o comando AT(...) avisa o GRUB para colocar o código na RAM Física em 1MB */
    
    .text ALIGN (0x1000) : AT(ADDR(.text) - 0xC0000000) {
        *(.text)
        *(.text*)
    }

    .rodata ALIGN (0x1000) : AT(ADDR(.rodata) - 0xC0000000) {
        *(.rodata)
        *(.rodata*)
    }

    .data ALIGN (0x1000) : AT(ADDR(.data) - 0xC0000000) {
        *(.data)
        *(.data*)
    }

    .bss ALIGN (0x1000) : AT(ADDR(.bss) - 0xC0000000) {
        *(COMMON)
        *(.bss)
        *(.bss*)
    }
}