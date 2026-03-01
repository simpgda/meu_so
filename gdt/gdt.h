#ifndef INCLUDE_GDT_H
#define INCLUDE_GDT_H

struct gdt {
    unsigned short size; // troquei a ordem 
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

void init_gdt(void);
extern void load_gdt(unsigned int gdt_ptr);

#endif /* INCLUDE_GDT_H */