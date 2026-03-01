#include "io.h"
#include "gdt.h"
#include "idt.h"

/* --- CONFIGURAÇÃO DO FRAMEBUFFER (Tela VGA) --- */
/* Portas de I/O usadas para enviar comandos para a placa de vídeo (VGA) */
#define FB_COMMAND_PORT         0x3D4
#define FB_DATA_PORT            0x3D5
#define FB_HIGH_BYTE_COMMAND    14
#define FB_LOW_BYTE_COMMAND     15

#define FB_GREEN     2
#define FB_DARK_GREY 8

/* O endereço físico 0x000B8000 é reservado pelo hardware do PC para o 
   VGA text buffer. O que escrevemos aqui aparece magicamente na tela. */
char *fb = (char *) 0x000B8000;

void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg)
{
    fb[i] = c;
    /* O byte de cor no VGA mistura o background (bg) nos 4 bits mais altos 
       e o foreground (fg) nos 4 bits mais baixos. */
    fb[i + 1] = ((fg & 0x0F) << 4) | (bg & 0x0F);
}

void fb_move_cursor(unsigned short pos)
{
    /* Diz para a placa VGA que vamos enviar o byte mais alto (high byte) da posição do cursor */
    outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    outb(FB_DATA_PORT,    ((pos >> 8) & 0x00FF));

    /* Diz para a placa VGA que vamos enviar o byte mais baixo (low byte) da posição do cursor */
    outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    outb(FB_DATA_PORT,    pos & 0x00FF);
}

/* --- CONFIGURAÇÃO DA PORTA SERIAL (Logs de Depuração) --- */
/* 0x3F8 é a porta base padrão do hardware para a COM1 (primeira porta serial) */
#define SERIAL_COM1_BASE                0x3F8      

#define SERIAL_DATA_PORT(base)          (base)
#define SERIAL_FIFO_COMMAND_PORT(base)  (base + 2)
#define SERIAL_LINE_COMMAND_PORT(base)  (base + 3)
#define SERIAL_MODEM_COMMAND_PORT(base) (base + 4)
#define SERIAL_LINE_STATUS_PORT(base)   (base + 5)

/* Flag para habilitar o DLAB (Divisor Latch Access Bit), necessário para configurar a velocidade (baud rate) */
#define SERIAL_LINE_ENABLE_DLAB         0x80

void serial_configure_baud_rate(unsigned short com, unsigned short divisor)
{
    outb(SERIAL_LINE_COMMAND_PORT(com),
         SERIAL_LINE_ENABLE_DLAB);
    outb(SERIAL_DATA_PORT(com),
         (divisor >> 8) & 0x00FF);
    outb(SERIAL_DATA_PORT(com),
         divisor & 0x00FF);
}

void serial_configure_line(unsigned short com)
{
    /* Bit:     | 7 | 6 | 5 4 3 | 2 | 1 0 |
     * Content: | d | b | prty  | s | dl  |
     * Value:   | 0 | 0 | 0 0 0 | 0 | 1 1 | = 0x03
     */

    /* Configura o protocolo da linha serial: 8 bits de dados, sem paridade, 1 stop bit (8N1) */
    outb(SERIAL_LINE_COMMAND_PORT(com), 0x03);
}

void serial_configure_fifo(unsigned short com) {
    /* Habilita a fila (FIFO), limpa as filas de envio/recebimento e define o tamanho para 14 bytes */
    outb(SERIAL_FIFO_COMMAND_PORT(com), 0xC7);
}

void serial_configure_modem(unsigned short com) {
    /* RTS = 1, DTS = 1 */
    /* Ready To Transmit (RTS) e Data Terminal Ready (DTR) configurados como 1 (pronto para uso) */
    outb(SERIAL_MODEM_COMMAND_PORT(com), 0x03);
}

int serial_is_transmit_fifo_empty(unsigned int com)
{
    /* 0x20 = 0010 0000 */
    /* Lê o registrador de status. O bit 5 (0x20) indica se a fila de transmissão está vazia */
    return inb(SERIAL_LINE_STATUS_PORT(com)) & 0x20;
}

void serial_write(char *buf, unsigned int len) {
    unsigned int i;
    for (i = 0; i < len; i++) {
        /* Espera ativamente (busy waiting) até a porta serial estar livre para enviar o próximo caractere */
        while (serial_is_transmit_fifo_empty(SERIAL_COM1_BASE) == 0);
        outb(SERIAL_DATA_PORT(SERIAL_COM1_BASE), buf[i]);
    }
}

/* --- KERNEL MAIN --- */
int kmain(void)
{
    /* Configura o Flat Memory Model e os anéis de segurança (Ring 0 / Kernel mode) */
    init_gdt();
    /* Carrega a tabela de interrupções para a CPU saber o que fazer quando o hardware chamar */
    init_idt();

    /* Desenha um 'A' verde na primeira posição da memória de vídeo para provar que o SO iniciou */
    fb_write_cell(0, 'A', FB_GREEN, FB_DARK_GREY);
    fb_move_cursor(1);

    /* Inicializa a porta serial para podermos enviar logs silenciosos que não sujam a tela do SO */
    serial_configure_baud_rate(SERIAL_COM1_BASE, 1);
    serial_configure_line(SERIAL_COM1_BASE);
    serial_configure_fifo(SERIAL_COM1_BASE);
    serial_configure_modem(SERIAL_COM1_BASE);

    char log_msg[] = "Hello Serial World!";
    serial_write(log_msg, sizeof(log_msg));

    return 0xCAFEBABE;
}