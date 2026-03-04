#include "io.h"
#include "serial.h"

/* * Endereços relativos das portas de controle da Serial.
 * Tudo é calculado a partir do endereço base da porta COM.
 */
#define SERIAL_DATA_PORT(base)          (base)
#define SERIAL_FIFO_COMMAND_PORT(base)  (base + 2)
#define SERIAL_LINE_COMMAND_PORT(base)  (base + 3)
#define SERIAL_MODEM_COMMAND_PORT(base) (base + 4)
#define SERIAL_LINE_STATUS_PORT(base)   (base + 5)

/* Bit necessário para configurar a velocidade (Baud Rate) */
#define SERIAL_LINE_ENABLE_DLAB 0x80

/* Define o quão rápido os dados viajam pelo cabo virtual */
void serial_configure_baud_rate(unsigned short com, unsigned short divisor) {
    outb(SERIAL_LINE_COMMAND_PORT(com), SERIAL_LINE_ENABLE_DLAB);
    outb(SERIAL_DATA_PORT(com), divisor & 0x00FF);
    outb(SERIAL_DATA_PORT(com), (divisor >> 8) & 0x00FF);
}

/* Padroniza a comunicação: 8 bits de dados, 1 bit de parada, sem paridade */
void serial_configure_line(unsigned short com) {
    outb(SERIAL_LINE_COMMAND_PORT(com), 0x03);
}

/* Ativa e limpa a fila de espera (FIFO) de envio e recebimento */
void serial_configure_buffers(unsigned short com) {
    outb(SERIAL_FIFO_COMMAND_PORT(com), 0xC7);
}

/* Informa ao emulador que o nosso SO está pronto para transmitir dados */
void serial_configure_modem(unsigned short com) {
    outb(SERIAL_MODEM_COMMAND_PORT(com), 0x03);
}

/* Checa se o cabo virtual já terminou de enviar o último caractere */
int serial_is_transmit_fifo_empty(unsigned int com) {
    return inb(SERIAL_LINE_STATUS_PORT(com)) & 0x20;
}

/* Empurra os caracteres um a um para fora do SO */
void serial_write(unsigned short com, char *buf, unsigned int len) {
    unsigned int i;
    for (i = 0; i < len; i++) {
        while (!serial_is_transmit_fifo_empty(com));
        outb(SERIAL_DATA_PORT(com), buf[i]);
    }
}

/* Função central que invoca todas as configurações acima na inicialização */
void serial_init(void) {
    serial_configure_baud_rate(SERIAL_COM1_BASE, 3);
    serial_configure_line(SERIAL_COM1_BASE);
    serial_configure_buffers(SERIAL_COM1_BASE);
    serial_configure_modem(SERIAL_COM1_BASE);
}