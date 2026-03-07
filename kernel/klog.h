#ifndef INCLUDE_KLOG_H
#define INCLUDE_KLOG_H

/*
 * klog.h - Sistema de Log do Kernel (Capítulo 4 - Driver de Saída)
 *
 * Fornece uma interface unificada para enviar mensagens de log
 * simultaneamente para dois destinos:
 *   1. Framebuffer (tela do emulador Bochs - visível para o usuário)
 *   2. Porta Serial COM1 (salva no arquivo com1.out - visível para o desenvolvedor)
 *
 * Uso:
 *   klog_init();                    // Inicializa serial + limpa tela
 *   klog_write("Mensagem aqui\n");  // Aparece na tela E no com1.out
 */

/* Inicializa a porta serial COM1 e limpa o framebuffer */
void klog_init(void);

/* Escreve uma string (terminada em '\0') na tela e na serial ao mesmo tempo */
void klog_write(const char *msg);

#endif
