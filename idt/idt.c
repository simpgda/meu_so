#include "idt.h"

struct idt_entry idt_entries[256];
struct idt_ptr   idt_pointer;

/* Handlers de exceções da CPU (0-31) */
extern void interrupt_handler_0(void);
extern void interrupt_handler_1(void);
extern void interrupt_handler_2(void);
extern void interrupt_handler_3(void);
extern void interrupt_handler_4(void);
extern void interrupt_handler_5(void);
extern void interrupt_handler_6(void);
extern void interrupt_handler_7(void);
extern void interrupt_handler_8(void);
extern void interrupt_handler_9(void);
extern void interrupt_handler_10(void);
extern void interrupt_handler_11(void);
extern void interrupt_handler_12(void);
extern void interrupt_handler_13(void);
extern void interrupt_handler_14(void);
extern void interrupt_handler_15(void);
extern void interrupt_handler_16(void);
extern void interrupt_handler_17(void);
extern void interrupt_handler_18(void);
extern void interrupt_handler_19(void);
extern void interrupt_handler_20(void);
extern void interrupt_handler_21(void);
extern void interrupt_handler_22(void);
extern void interrupt_handler_23(void);
extern void interrupt_handler_24(void);
extern void interrupt_handler_25(void);
extern void interrupt_handler_26(void);
extern void interrupt_handler_27(void);
extern void interrupt_handler_28(void);
extern void interrupt_handler_29(void);
extern void interrupt_handler_30(void);
extern void interrupt_handler_31(void);

/* IRQs de hardware */
extern void interrupt_handler_33(void);
extern void interrupt_handler_39(void);

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

    /* Exceções da CPU (0-31) */
    idt_set_gate(0,  (unsigned int)interrupt_handler_0,  0x08, 0x8E);
    idt_set_gate(1,  (unsigned int)interrupt_handler_1,  0x08, 0x8E);
    idt_set_gate(2,  (unsigned int)interrupt_handler_2,  0x08, 0x8E);
    idt_set_gate(3,  (unsigned int)interrupt_handler_3,  0x08, 0x8E);
    idt_set_gate(4,  (unsigned int)interrupt_handler_4,  0x08, 0x8E);
    idt_set_gate(5,  (unsigned int)interrupt_handler_5,  0x08, 0x8E);
    idt_set_gate(6,  (unsigned int)interrupt_handler_6,  0x08, 0x8E);
    idt_set_gate(7,  (unsigned int)interrupt_handler_7,  0x08, 0x8E);
    idt_set_gate(8,  (unsigned int)interrupt_handler_8,  0x08, 0x8E);
    idt_set_gate(9,  (unsigned int)interrupt_handler_9,  0x08, 0x8E);
    idt_set_gate(10, (unsigned int)interrupt_handler_10, 0x08, 0x8E);
    idt_set_gate(11, (unsigned int)interrupt_handler_11, 0x08, 0x8E);
    idt_set_gate(12, (unsigned int)interrupt_handler_12, 0x08, 0x8E);
    idt_set_gate(13, (unsigned int)interrupt_handler_13, 0x08, 0x8E);
    idt_set_gate(14, (unsigned int)interrupt_handler_14, 0x08, 0x8E);
    idt_set_gate(15, (unsigned int)interrupt_handler_15, 0x08, 0x8E);
    idt_set_gate(16, (unsigned int)interrupt_handler_16, 0x08, 0x8E);
    idt_set_gate(17, (unsigned int)interrupt_handler_17, 0x08, 0x8E);
    idt_set_gate(18, (unsigned int)interrupt_handler_18, 0x08, 0x8E);
    idt_set_gate(19, (unsigned int)interrupt_handler_19, 0x08, 0x8E);
    idt_set_gate(20, (unsigned int)interrupt_handler_20, 0x08, 0x8E);
    idt_set_gate(21, (unsigned int)interrupt_handler_21, 0x08, 0x8E);
    idt_set_gate(22, (unsigned int)interrupt_handler_22, 0x08, 0x8E);
    idt_set_gate(23, (unsigned int)interrupt_handler_23, 0x08, 0x8E);
    idt_set_gate(24, (unsigned int)interrupt_handler_24, 0x08, 0x8E);
    idt_set_gate(25, (unsigned int)interrupt_handler_25, 0x08, 0x8E);
    idt_set_gate(26, (unsigned int)interrupt_handler_26, 0x08, 0x8E);
    idt_set_gate(27, (unsigned int)interrupt_handler_27, 0x08, 0x8E);
    idt_set_gate(28, (unsigned int)interrupt_handler_28, 0x08, 0x8E);
    idt_set_gate(29, (unsigned int)interrupt_handler_29, 0x08, 0x8E);
    idt_set_gate(30, (unsigned int)interrupt_handler_30, 0x08, 0x8E);
    idt_set_gate(31, (unsigned int)interrupt_handler_31, 0x08, 0x8E);

    /* IRQs de hardware */
    idt_set_gate(33, (unsigned int)interrupt_handler_33, 0x08, 0x8E);
    idt_set_gate(39, (unsigned int)interrupt_handler_39, 0x08, 0x8E);

    load_idt((unsigned int)&idt_pointer);
}