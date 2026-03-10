#include "io.h"

void pic_remap(void) {
    // Inicia a reconfiguração dos PICs
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    
    // Mapeia o PIC1 para a interrupção 32 (O teclado vai ser o 33)
    outb(0x21, 0x20); 
    // Mapeia o PIC2 para a interrupção 40
    outb(0xA1, 0x28); 
    
    // Configura a cascata entre eles
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    
    // Define o modo 8086
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    
    // Desmascara apenas a IRQ1 (Teclado) e bloqueia o resto por enquanto
    outb(0x21, 0xFD); 
    outb(0xA1, 0xFF);
}