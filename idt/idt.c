#include "idt.h"

struct idt_entry idt_entries[256];
struct idt_ptr   idt_pointer;

extern void interrupt_handler_33(void);

void idt_set_gate(unsigned char num, unsigned int base, unsigned short sel, unsigned char flags)
{
    idt_entries[num].offset_low  = (base & 0xFFFF);
    idt_entries[num].offset_high = (base >> 16) & 0xFFFF;
    idt_entries[num].selector    = sel;
    idt_entries[num].zero        = 0;
    idt_entries[num].type_attr   = flags;
}

void init_idt(void)
{
    idt_pointer.limit = sizeof(struct idt_entry) * 256 - 1;
    idt_pointer.base  = (unsigned int)&idt_entries;

    /* 0x08 = Code Segment, 0x8E = Interrupt Gate */
    idt_set_gate(33, (unsigned int)interrupt_handler_33, 0x08, 0x8E);

    load_idt((unsigned int)&idt_pointer);
}