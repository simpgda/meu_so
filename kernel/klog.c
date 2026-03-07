/*
 * klog.c - Implementação do Sistema de Log do Kernel
 *
 * Este arquivo conecta os dois drivers de saída (framebuffer e serial)
 * em uma única função klog_write(). Qualquer mensagem enviada por aqui
 * aparece simultaneamente:
 *   - Na tela do emulador (framebuffer em 0xB8000)
 *   - No arquivo com1.out (porta serial COM1 em 0x3F8)
 */

#include "klog.h"
#include "fb.h"
#include "serial.h"

/* Porta serial usada para log (COM1 = 0x3F8) */
#define KLOG_COM_PORT SERIAL_COM1_BASE

/*
 * klog_strlen:
 * Calcula o comprimento de uma string terminada em '\0'.
 * Precisamos disso porque não temos a biblioteca padrão (strlen não existe aqui).
 */
static unsigned int klog_strlen(const char *s) {
    unsigned int len = 0;
    while (s && s[len] != '\0') {
        len++;
    }
    return len;
}

/*
 * klog_init:
 * Prepara os dois canais de saída:
 *   1. serial_init() configura velocidade, formato e buffers da COM1
 *   2. fb_clear() preenche a tela inteira com espaços (fundo preto)
 */
void klog_init(void) {
    serial_init();
    fb_clear();
}

/*
 * klog_write:
 * Envia a mensagem para a tela e para a serial.
 * A string precisa terminar em '\0'. Caracteres especiais
 * como '\n' (nova linha) e '\b' (backspace) são tratados pelo fb_write.
 */
void klog_write(const char *msg) {
    unsigned int len = klog_strlen(msg);
    if (len == 0) {
        return;
    }
    fb_write((char *)msg, len);                    /* Escreve na tela        */
    serial_write(KLOG_COM_PORT, (char *)msg, len); /* Escreve no com1.out    */
}
