/*
 * kmain.c - Função Principal do Kernel (Capítulos 3-7)
 *
 * Este é o "main" do nosso Sistema Operacional. O GRUB carrega o kernel,
 * o loader.s configura a pilha e passa o controle para cá.
 *
 * Fluxo de execução:
 *   1. Configura GDT (segmentação) e IDT (interrupções)
 *   2. Remapeia o PIC para evitar conflito com exceções da CPU
 *   3. Habilita interrupções (sti)
 *   4. Inicializa os drivers de saída (tela + serial)
 *   5. Valida e executa o módulo externo carregado pelo GRUB (Cap. 7)
 *   6. Entra em loop infinito aguardando interrupções de hardware
 */

#include "gdt.h"
#include "idt.h"
#include "klog.h"
#include "multiboot.h"
#include "pic.h"

/*
 * kmain:
 * Ponto de entrada principal do kernel, chamado pelo loader.s.
 *
 * @param multiboot_info_addr  Endereço da struct multiboot_info preenchida pelo GRUB.
 *                             O GRUB coloca esse ponteiro no registrador EBX,
 *                             e o loader.s empurra na pilha antes de chamar kmain.
 */
int kmain(unsigned int multiboot_info_addr)
{
    /* === FASE 1: Inicialização do Hardware da CPU === */

    /* Carrega a GDT (Global Descriptor Table) - define os segmentos de memória (Cap. 5) */
    init_gdt();

    /* Carrega a IDT (Interrupt Descriptor Table) - registra os handlers de interrupção (Cap. 6) */
    init_idt();
    
    /* Remapeia o PIC: move IRQs do hardware para interrupções 32-47 (Cap. 6) */
    pic_remap();
    
    /* STI (Set Interrupt Flag): a partir daqui a CPU aceita interrupções externas */
    __asm__("sti");
    
    /* === FASE 2: Inicialização dos Drivers de Saída === */

    /* Configura a serial COM1 e limpa a tela do framebuffer (Cap. 4) */
    klog_init();

    /* Mensagens de boot - aparecem na tela do Bochs E no arquivo com1.out */
    klog_write("Log: Sistema Operacional Iniciado com Sucesso!\n");
    klog_write("Sistema Operacional Carregado!\n");
    klog_write("[OK] Hardware configurado.\n");

    /* === FASE 3: Carregamento e Execução do Módulo Externo (Cap. 7) === */

    /* Converte o endereço passado pelo GRUB (via EBX) para a struct multiboot_info */
    struct multiboot_info *mbi = (struct multiboot_info *)multiboot_info_addr;

    /* Verifica se o GRUB realmente carregou exatamente 1 módulo */
    if (!(mbi->flags & MULTIBOOT_INFO_MODS) || mbi->mods_count != 1) {
        klog_write("Erro: modulo GRUB ausente.\n");
        while (1) {
            __asm__("hlt");
        }
    }

    /* Pega o endereço de memória onde o GRUB colocou o binário do módulo */
    struct multiboot_module *mod = (struct multiboot_module *)mbi->mods_addr;
    klog_write("Executando modulo externo...\n");

    /*
     * Cria um ponteiro de função apontando para o início do módulo na memória.
     * Quando chamamos start_program(), a CPU pula para o código do programa externo.
     * Se o programa fizer 'ret', o controle volta para cá.
     */
    typedef void (*call_module_t)(void);
    call_module_t start_program = (call_module_t)mod->mod_start;
    start_program();

    /* Só chega aqui se o módulo executar 'ret' (retornar ao kernel) */
    klog_write("Modulo retornou ao kernel.\n");
    klog_write("Pode digitar agora...\n> ");

    /* === FASE 4: Loop Principal === */

    /* O kernel adormece e só acorda quando uma interrupção chega (ex: tecla pressionada) */
    while(1) {
        __asm__("hlt");
    }

    return 0;
}