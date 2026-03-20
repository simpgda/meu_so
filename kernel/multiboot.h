#ifndef INCLUDE_MULTIBOOT_H
#define INCLUDE_MULTIBOOT_H

/*
 * multiboot.h - Estruturas do padrão Multiboot (Capítulo 7)
 *
 * Quando o GRUB carrega o kernel, ele preenche uma estrutura na memória
 * com informações sobre o hardware e os módulos carregados.
 * O ponteiro para essa estrutura é passado no registrador EBX.
 *
 * Referência: https://www.gnu.org/software/grub/manual/multiboot/multiboot.html
 */

/* Bit 3 do campo 'flags': indica que informações sobre módulos estão disponíveis */
#define MULTIBOOT_INFO_MODS 0x00000008
/* Bit 6 do campo 'flags': indica que informações de mapa de memória estão disponíveis */
#define MULTIBOOT_INFO_MMAP 0x00000040

/*
 * multiboot_info:
 * Estrutura principal preenchida pelo GRUB antes de passar o controle ao kernel.
 * Contém dados sobre memória, dispositivo de boot, linha de comando e módulos.
 */
struct multiboot_info {
    unsigned int flags;        /* Bits indicando quais campos abaixo são válidos       */
    unsigned int mem_lower;    /* Quantidade de memória abaixo de 1 MB (em KB)         */
    unsigned int mem_upper;    /* Quantidade de memória acima de 1 MB (em KB)          */
    unsigned int boot_device;  /* Dispositivo de boot usado pelo GRUB                  */
    unsigned int cmdline;      /* Endereço da string de linha de comando do kernel      */
    unsigned int mods_count;   /* Quantos módulos o GRUB carregou                      */
    unsigned int mods_addr;    /* Endereço do array de structs multiboot_module         */
    unsigned int syms[4];      /* Informações sobre símbolos                           */
    unsigned int mmap_length;  /* Tamanho total do mapa de memória (em bytes)          */
    unsigned int mmap_addr;    /* Endereço físico do primeiro item do mmap             */
} __attribute__((packed));

/*
 * multiboot_mmap_entry:
 * Descreve uma região de memória no mapa fornecido pelo GRUB.
 */
struct multiboot_mmap_entry {
    unsigned int size;         /* Tamanho da struct sem este campo (normalmente 20)    */
    unsigned int addr_low;     /* Endereço inicial (baixa 32-bit)                      */
    unsigned int addr_high;    /* Endereço inicial (alta 32-bit)                       */
    unsigned int len_low;      /* Tamanho da região (baixa 32-bit)                     */
    unsigned int len_high;     /* Tamanho da região (alta 32-bit)                      */
    unsigned int type;         /* Tipo: 1 = RAM disponível, Outros = Reservado         */
} __attribute__((packed));

/*
 * multiboot_module:
 * Descreve um módulo carregado pelo GRUB (ex: nosso programa externo).
 * O GRUB coloca o binário na memória entre mod_start e mod_end.
 */
struct multiboot_module {
    unsigned int mod_start;    /* Endereço de início do módulo na memória              */
    unsigned int mod_end;      /* Endereço de fim do módulo na memória                 */
    unsigned int string;       /* Endereço da string com o nome/caminho do módulo      */
    unsigned int reserved;     /* Reservado (não usar)                                 */
} __attribute__((packed));

#endif
