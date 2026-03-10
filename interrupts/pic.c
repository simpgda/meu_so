/*
 * pic.c - Implementação do Controlador de Interrupções (PIC 8259)
 *
 * Centraliza todo o código relacionado ao PIC num único arquivo.
 * Antes estava espalhado entre kmain.c (pic_remap) e interrupts.c (pic_acknowledge).
 */

#include "pic.h"
#include "io.h"

/*
 * pic_remap:
 * O processador x86 reserva as interrupções 0-31 para erros internos (ex: divisão por zero).
 * O hardware (PIC) vem de fábrica usando esses mesmos números, o que causa conflito.
 * Esta função empurra as interrupções de hardware para frente (começando no 32)
 * e aplica uma máscara para escutar APENAS o teclado.
 *
 * Sequência ICW (Initialization Command Words):
 *   ICW1 (0x11): Inicia a sequência de configuração
 *   ICW2: Novo offset (Master=32, Slave=40)
 *   ICW3: Cascata (Master: slave no pino 2, Slave: conectado ao pino 2)
 *   ICW4 (0x01): Modo 8086
 */
void pic_remap(void) {
    /* ICW1: Reinicia os controladores Master e Slave */
    outb(PIC1_PORT_A, 0x11);
    outb(PIC2_PORT_A, 0x11);

    /* ICW2: Define o novo ponto de partida: Master no 32, Slave no 40 */
    outb(PIC1_PORT_B, 0x20);
    outb(PIC2_PORT_B, 0x28);

    /* ICW3: Configura a cascata de comunicação entre os dois chips */
    outb(PIC1_PORT_B, 0x04);
    outb(PIC2_PORT_B, 0x02);

    /* ICW4: Modo 8086 */
    outb(PIC1_PORT_B, 0x01);
    outb(PIC2_PORT_B, 0x01);

    /* Máscara: 0xFD (11111101) libera só o bit 1 (Teclado). 0xFF silencia o Slave inteiro. */
    outb(PIC1_PORT_B, 0xFD);
    outb(PIC2_PORT_B, 0xFF);
}

/*
 * pic_acknowledge:
 * Após tratar uma interrupção de hardware, precisamos avisar o PIC
 * que já terminamos o trabalho (enviar EOI - End Of Interrupt).
 * Se a IRQ veio do Slave (>= 40), precisamos avisar os dois PICs.
 */
void pic_acknowledge(unsigned int interrupt) {
    /* Ignora interrupções fora do intervalo de IRQs (32-47) */
    if (interrupt < 0x20 || interrupt > 0x2F) {
        return;
    }

    /* Se veio do Slave (IRQs 40-47), envia EOI para ele também */
    if (interrupt >= 0x28) {
        outb(PIC2_PORT_A, PIC_ACK);
    }

    /* Sempre envia EOI para o Master */
    outb(PIC1_PORT_A, PIC_ACK);
}
