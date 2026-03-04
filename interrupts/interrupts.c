#include "idt.h"
#include "fb.h"
#include "serial.h"
#include "io.h"

/* ========================================================================= *
 * FUNÇÕES DE HARDWARE (Trazidas para cá para simplificar)                   *
 * ========================================================================= */
#define PIC1_PORT_A 0x20
#define PIC2_PORT_A 0xA0
#define PIC_ACK     0x20

void pic_acknowledge(unsigned int interrupt) {
    if (interrupt < 0x20 || interrupt > 0x2F) return;
    if (interrupt >= 0x28) outb(PIC2_PORT_A, PIC_ACK);
    outb(PIC1_PORT_A, PIC_ACK);
}

unsigned char read_scan_code(void) {
    return inb(0x60);
}

/* ========================================================================= *
 * LÓGICA DO TECLADO                                                         *
 * ========================================================================= */
unsigned char kbd_map[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

void interrupt_handler(struct cpu_state cpu, unsigned int interrupt, struct stack_state stack) {
    (void)cpu; (void)stack;

    if (interrupt == 33) {
        unsigned char scancode = read_scan_code();
        
        if (!(scancode & 0x80)) {
            char letra = kbd_map[scancode];
            if (letra != 0) {
                fb_write(&letra, 1);
                serial_write(0x3F8, &letra, 1); /* 0x3F8 é a porta COM1 */
            }
        }
        pic_acknowledge(interrupt);
    }
}