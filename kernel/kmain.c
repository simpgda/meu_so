#include "gdt.h"
#include "idt.h"
#include "fb.h"
#include "serial.h"
#include "io.h"

/*
 * pic_remap:
 * O processador x86 reserva as interrupções 0-31 para erros internos (ex: divisão por zero).
 * O hardware (PIC) vem de fábrica usando esses mesmos números, o que causa conflito.
 * Esta função empurra as interrupções de hardware para frente (começando no 32)
 * e aplica uma máscara para escutar APENAS o teclado.
 */
void pic_remap(void) {
    /* Reinicia os controladores Master e Slave */
    outb(0x20, 0x11); outb(0xA0, 0x11);
    
    /* Define o novo ponto de partida: Master no 32 (0x20) e Slave no 40 (0x28) */
    outb(0x21, 0x20); outb(0xA1, 0x28); 
    
    /* Configura a cascata de comunicação entre os dois chips */
    outb(0x21, 0x04); outb(0xA1, 0x02);
    outb(0x21, 0x01); outb(0xA1, 0x01);
    
    /* Máscara: 0xFD (11111101) libera só o bit 1 (Teclado). 0xFF silencia o Slave inteiro. */
    outb(0x21, 0xFD); 
    outb(0xA1, 0xFF);
}

int kmain(void)
{
    /* Prepara as estruturas de memória e interrupções da CPU */
    init_gdt();
    init_idt();
    
    /* Organiza os hardwares externos para não travarem a CPU */
    pic_remap();
    
    /* Habilita a CPU para começar a responder a eventos externos (como digitar) */
    __asm__("sti");
    
    /* Prepara os dispositivos de saída (Arquivo de Log e Tela do Emulador) */
    serial_init();
    fb_clear();

    /* Envia uma mensagem oculta para a porta serial (verificável no com1.out) */
    char log_boot[] = "Log: Sistema Operacional Iniciado com Sucesso!\n";
    serial_write(0x3F8, log_boot, 47);

    /* Renderiza a interface inicial para o usuário no Framebuffer */
    fb_write("Sistema Operacional Carregado!\n", 31);
    fb_write("[OK] Hardware configurado.\n", 27);
    fb_write("Pode digitar agora...\n> ", 24);  

    /* O SO adormece aguardando a próxima interrupção de hardware para economizar CPU */
    while(1) {
        __asm__("hlt");
    }

    return 0;
}