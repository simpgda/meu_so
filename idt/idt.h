#ifndef INCLUDE_IDT_H
#define INCLUDE_IDT_H

struct idt_entry {
    unsigned short offset_low;  /* 0-15 bits */
    unsigned short selector;    /* 16-31 bits */
    unsigned char  zero;        /* 32-39 bits (unused) */
    unsigned char  type_attr;   /* 40-47 bits (type and attributes) */
    unsigned short offset_high; /* 48-63 bits */
} __attribute__((packed));

struct idt_ptr {
    unsigned short limit;
    unsigned int   base;
} __attribute__((packed));

/*
 * cpu_state:
 * Representa os registradores salvos pelo handler de interrupção.
 * A ORDEM IMPORTA: precisa ser a inversa dos pushes no Assembly.
 * O último push (edi) vira o primeiro campo, porque a pilha cresce para baixo.
 */
struct cpu_state {
    unsigned int edi;
    unsigned int esi;
    unsigned int ebp;
    unsigned int edx;
    unsigned int ecx;
    unsigned int ebx;
    unsigned int eax;
} __attribute__((packed));

struct stack_state {
    unsigned int error_code;
    unsigned int eip;
    unsigned int cs;
    unsigned int eflags;
} __attribute__((packed));

void init_idt(void);
extern void load_idt(unsigned int idt_ptr);

#endif /* INCLUDE_IDT_H */