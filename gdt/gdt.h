#ifndef INCLUDE_GDT_H
#define INCLUDE_GDT_H

struct gdt {
    unsigned short size; 
    unsigned int address;
} __attribute__((packed));

struct gdt_entry {
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char  base_middle;
    unsigned char  access;
    unsigned char  granularity;
    unsigned char  base_high;
} __attribute__((packed));

// Padronizado para init_gdt
void init_gdt(void); 
void gdt_set_gate(int num, unsigned int base, unsigned int limit, unsigned char access, unsigned char gran);

// O Assembly usa gdt_flush, então o C tem que chamar gdt_flush!
extern void gdt_flush(unsigned int gdt_ptr);

#endif /* INCLUDE_GDT_H */