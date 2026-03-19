#include "io.h"
#include "fb.h"

/* Endereços de hardware para conversação com a placa VGA */
#define FB_COMMAND_PORT 0x3D4
#define FB_DATA_PORT    0x3D5
#define FB_HIGH_BYTE    14
#define FB_LOW_BYTE     15

/* O Framebuffer usa o endereço virtual mapeado na metade superior (Higher Half) */
static char *fb = (char *) 0xC00B8000;

/* Dimensões da tela em modo texto VGA */
#define FB_COLS 80
#define FB_ROWS 25
#define FB_SIZE (FB_COLS * FB_ROWS)

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
    for (i = 0; i < FB_SIZE; i++) {
        fb_write_cell(i, ' ', 15, 0);
    }
    cursor_pos = 0;
    fb_move_cursor(cursor_pos);
}

/*
 * fb_scroll:
 * Quando o cursor ultrapassa a última linha (posição >= 2000),
 * copia todas as linhas uma posição para cima e limpa a última linha.
 * Isso evita que o texto "saia" da tela.
 */
static void fb_scroll(void) {
    unsigned int i;

    if (cursor_pos < FB_SIZE) {
        return;
    }

    /* Copia cada célula (2 bytes: caractere + cor) uma linha para cima */
    for (i = 0; i < (FB_ROWS - 1) * FB_COLS * 2; i++) {
        fb[i] = fb[i + FB_COLS * 2];
    }

    /* Limpa a última linha com espaços */
    for (i = (FB_ROWS - 1) * FB_COLS; i < FB_SIZE; i++) {
        fb_write_cell(i, ' ', 15, 0);
    }

    /* Reposiciona o cursor no início da última linha */
    cursor_pos = (FB_ROWS - 1) * FB_COLS;
}

/* Função inteligente que traduz frases, quebras de linha e backspaces para a tela */
void fb_write(char *str, unsigned int len) {
    unsigned int i;
    for (i = 0; i < len; i++) {
        if (str[i] == '\n') {
            /* Avança o cursor para o primeiro caractere da próxima linha múltipla de 80 */
            cursor_pos = ((cursor_pos / FB_COLS) + 1) * FB_COLS;
            
        } else if (str[i] == '\t') {
            /* Tab: avança o cursor até a próxima posição múltipla de 4 */
            unsigned short tab_stop = (cursor_pos + 4) & ~3;
            while (cursor_pos < tab_stop) {
                fb_write_cell(cursor_pos, ' ', 15, 0);
                cursor_pos++;
                fb_scroll();
            }
            /* O scroll e avanço do loop principal não são necessários para tab */
            continue;

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

        /* Se o cursor passou da tela, rola tudo uma linha para cima */
        fb_scroll();
    }
    fb_move_cursor(cursor_pos); 
}