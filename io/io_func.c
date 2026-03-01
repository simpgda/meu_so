#include "io.h"

/* Portas de I/O usadas pelo chip controlador de interrupções (PIC) 
   O PIC Master atende na porta 0x20 e o PIC Slave atende na porta 0xA0. */
#define PIC1_PORT_A 0x20
#define PIC2_PORT_A 0xA0

/* Remapeamos as interrupções de hardware para começar em 32 (0x20), 
   pois de 0 a 31 são reservadas para exceções internas da CPU (ex: divisão por zero). */
#define PIC1_START_INTERRUPT 0x20
#define PIC2_START_INTERRUPT 0x28
#define PIC2_END_INTERRUPT   PIC2_START_INTERRUPT + 7

/* Código 0x20 é o sinal de "End of Interrupt" (EOI) que devemos mandar ao PIC */
#define PIC_ACK 0x20

/* O hardware do teclado sempre envia a tecla pressionada para a porta 0x60 */
#define KBD_DATA_PORT 0x60

void pic_acknowledge(unsigned int interrupt)
{
    if (interrupt < PIC1_START_INTERRUPT || interrupt > PIC2_END_INTERRUPT) {
        return;
    }

    /* Se a interrupção veio do PIC Slave (teclado não é, mas outros hardwares sim), 
       precisamos mandar o ACK tanto para o Slave quanto para o Master. */
    if (interrupt < PIC2_START_INTERRUPT) {
        outb(PIC1_PORT_A, PIC_ACK);
    } else {
        outb(PIC2_PORT_A, PIC_ACK);
        outb(PIC1_PORT_A, PIC_ACK);
    }
}

unsigned char read_scan_code(void)
{
    /* Lê exatamente a porta onde o microcontrolador do teclado deixa o byte da tecla apertada */
    return inb(KBD_DATA_PORT);
}