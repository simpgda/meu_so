#include "gdt.h"

struct gdt_entry gdt_entries[3];
struct gdt       gdt_ptr;

void gdt_set_gate(int num, unsigned int base, unsigned int limit, unsigned char access, unsigned char gran)
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

    gdt_set_gate(0, 0, 0, 0, 0); // Null
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data

    gdt_flush((unsigned int)&gdt_ptr);
}