#include "gdt.h"

// Força o alinhamento em 4 bytes para garantir que o processador 
// não se perca ao ler a estrutura na memória.
__attribute__((aligned(4))) struct gdt_entry gdt_entries[3];
__attribute__((aligned(4))) struct gdt       gdt_ptr;

void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran)
{
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low   = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F;

    gdt_entries[num].granularity |= gran & 0xF0;
    gdt_entries[num].access      = access;
}

void init_gdt(void)
{
    gdt_ptr.size = (sizeof(struct gdt_entry) * 3) - 1;
    gdt_ptr.address = (unsigned int)&gdt_entries;

    
    /* 1. Null Descriptor (Index 0) */
    gdt_set_gate(0, 0, 0, 0, 0);

    /* 2. Code Segment (Index 1, Offset 0x08) 
     * Base: 0, Limit: 0xFFFFFFFF (4GB)
     * Access: 0x9A (Present, Ring 0, Code, Exec/Read)
     * Granularity: 0xCF (4KB blocks, 32-bit)
     */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    /* 3. Data Segment (Index 2, Offset 0x10)
     * Base: 0, Limit: 0xFFFFFFFF (4GB)
     * Access: 0x92 (Present, Ring 0, Data, Read/Write)
     * Granularity: 0xCF (4KB blocks, 32-bit)
     */
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    load_gdt((unsigned int)&gdt_ptr);
}