#include "idt.h"
#include "io.h"

extern void fb_move_cursor(unsigned short pos);

extern unsigned int cursor_pos;
static int shift_ativado = 0;

const unsigned char teclado_mapa[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',
  '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
  '*', 0, ' ', 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const unsigned char teclado_mapa_shift[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*',
  '(', ')', '_', '+', '\b',
  '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
  '*', 0, ' ', 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void interrupt_handler(struct cpu_state cpu, unsigned int interrupt, struct stack_state stack)
{
    (void)cpu; 
    (void)stack;

    if (interrupt == 33) {
        unsigned char scancode = inb(0x60);

        if (scancode == 0x2A || scancode == 0x36) {
            shift_ativado = 1;
        } 
        else if (scancode == 0xAA || scancode == 0xB6) {
            shift_ativado = 0;
        } 
        else if (scancode < 128) {
            char letra;
            if (shift_ativado) {
                letra = teclado_mapa_shift[scancode];
            } else {
                letra = teclado_mapa[scancode];
            }
            
            unsigned short *video_memory = (unsigned short *)0xB8000;
            
            if (letra != 0) {
                if (letra == '\b') {
                    if (cursor_pos > 0) {
                        cursor_pos--;
                        /* 0x70 = Fundo Cinza (7), Letra Preta (0) */
                        video_memory[cursor_pos] = (0x70 << 8) | ' ';
                    }
                } 
                else if (letra == '\n') {
                    cursor_pos = cursor_pos + 80 - (cursor_pos % 80);
                } 
                else {
                    /* AQUI O AJUSTE: Usamos 0x70 para combinar com o fundo cinza */
                    video_memory[cursor_pos] = (0x70 << 8) | letra;
                    cursor_pos++;
                }

                if (cursor_pos >= 2000) cursor_pos = 0; 
                fb_move_cursor(cursor_pos);
            }
        }

        outb(0x20, 0x20);
    }
}