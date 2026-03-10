#ifndef INCLUDE_PIC_H
#define INCLUDE_PIC_H

/*
 * pic.h - Controlador de Interrupções Programável (PIC 8259)
 *
 * O PIC é o chip que gerencia as interrupções de hardware (teclado, timer, etc.).
 * Numa placa x86 existem dois PICs encadeados (Master e Slave), totalizando 16 IRQs.
 *
 * Portas de comunicação:
 *   Master: 0x20 (comando) e 0x21 (dados)
 *   Slave:  0xA0 (comando) e 0xA1 (dados)
 */

/* Portas de comando dos dois PICs */
#define PIC1_PORT_A 0x20   /* Master - porta de comando */
#define PIC1_PORT_B 0x21   /* Master - porta de dados   */
#define PIC2_PORT_A 0xA0   /* Slave  - porta de comando */
#define PIC2_PORT_B 0xA1   /* Slave  - porta de dados   */

/* Byte de reconhecimento (End Of Interrupt) enviado ao PIC após tratar uma IRQ */
#define PIC_ACK     0x20

/*
 * pic_remap:
 * Remapeia as IRQs de hardware para interrupções 32-47.
 * Sem isso, as IRQs colidem com as exceções internas da CPU (0-31).
 */
void pic_remap(void);

/*
 * pic_acknowledge:
 * Envia o sinal de "tratei essa interrupção" (EOI) para o PIC correto.
 * Deve ser chamado no final de todo handler de IRQ.
 */
void pic_acknowledge(unsigned int interrupt);

#endif
