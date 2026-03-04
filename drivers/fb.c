#include "io.h"
#include "fb.h"

/* Endereços de hardware para conversação com a placa VGA */
#define FB_COMMAND_PORT 0x3D4
#define FB_DATA_PORT    0x3D5
#define FB_HIGH_BYTE    14
#define FB_LOW_BYTE     15

/* O Framebuffer mapeia a tela diretamente na memória RAM neste endereço físico */
static char *fb = (char *) 0x000B8000;

/* Índice que controla em qual das 2000 posições da tela (80x25) estamos escrevendo */
static unsigned short cursor_pos = 0;

/* Atualiza a posição daquele "tracinho" piscante nativo da placa de vídeo */
void fb_move_cursor(unsigned short pos) {
    outb(FB_COMMAND_PORT, FB_HIGH_BYTE);
    outb(FB_DATA_PORT, ((pos >> 8) & 0x00FF));
    outb(FB_COMMAND_PORT, FB_LOW_BYTE);
    outb(FB_DATA_PORT, pos & 0x00FF);
}

/* * Função mais baixa de vídeo: injeta os bytes diretamente na memória.
 * Pos multiplica por 2 porque cada caractere exige 2 bytes: [Letra] + [Cor].
 */
void fb_write_cell(unsigned int pos, char c, unsigned char fg, unsigned char bg) {
    unsigned int offset = pos * 2;
    fb[offset] = c;
    fb[offset + 1] = ((bg & 0x0F) << 4) | (fg & 0x0F);
}

/* "Varre" a memória de vídeo inteira pintando espaços com fundo preto */
void fb_clear(void) {
    unsigned int i;
    for (i = 0; i < 80 * 25; i++) {
        fb_write_cell(i, ' ', 15, 0);
    }
    cursor_pos = 0;
    fb_move_cursor(cursor_pos);
}

/* Função inteligente que traduz frases, quebras de linha e backspaces para a tela */
void fb_write(char *str, unsigned int len) {
    unsigned int i;
    for (i = 0; i < len; i++) {
        if (str[i] == '\n') {
            /* Avança o cursor para o primeiro caractere da próxima linha múltipla de 80 */
            cursor_pos = ((cursor_pos / 80) + 1) * 80;
            
        } else if (str[i] == '\b') {
            /* Impede que o cursor apague o que está fora da tela recuando além do zero */
            if (cursor_pos > 0) {
                cursor_pos--;
                fb_write_cell(cursor_pos, ' ', 15, 0); 
            }
            
        } else {
            /* Desenha o caractere (15 = Branco, 0 = Preto) e anda pra frente */
            fb_write_cell(cursor_pos, str[i], 15, 0);  
            cursor_pos++;
        }
    }
    fb_move_cursor(cursor_pos); 
}