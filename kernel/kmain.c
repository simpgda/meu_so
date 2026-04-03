/*
 * kmain.c - Função Principal do Kernel (Capítulos 3-9 e Tarefa 05)
 *
 * Este é o "main" do nosso Sistema Operacional. O GRUB carrega o kernel,
 * o loader.s configura a paginação (Higher Half) e passa o controle para cá.
 */

#include "gdt.h"
#include "idt.h"
#include "klog.h"
#include "multiboot.h"
#include "pic.h"
#include "../mm/pmm.h"
#include "../mm/vmm.h"
#include "../mm/kheap.h"
#include "task.h"
#include "sync.h" 

// ========================================================
// TAREFA 05: CHAT MULTI-USUÁRIO E DEADLOCK
// ========================================================

 
// 0 = Sem Hierarquia de Travas (com DeadLock), 1 = Com Hierarquia de Travas (sem DeadLock)
int modo_seguro = 1; 

// Primitivas de sincronização para garantir a Exclusão Mútua no acesso aos recursos compartilhados (Buffer de Log e Arquivos simulados).
mutex_t chat_mutex;       
mutex_t arquivo_A_mutex;  
mutex_t arquivo_B_mutex;  

// Nossas Threads dos Clientes e do kmain (só para salvar o estado da pilha)
thread_t thread_c1;
thread_t thread_c2;
thread_t thread_kmain; 

void delay() {
    for(volatile int i = 0; i < 15000000; i++);
}

// --- CLIENTE 1 (O Padrão) ---
void cliente_1() {
    while(1) {
        // PARTE 1: O Chat
        mutex_lock(&chat_mutex);
        klog_write("\n[Chat] Cliente 1: Te enviei um arquivo, Cliente 2!\n");
        mutex_unlock(&chat_mutex);
        delay();

        // PARTE 2: Pede A, depois B
        klog_write("[C1] Trancando Arquivo A...\n");
        mutex_lock(&arquivo_A_mutex);
        
        task_yield(); // Passa a vez
        
        klog_write("[C1] Tentando trancar Arquivo B...\n");
        mutex_lock(&arquivo_B_mutex); 
        
        // Se chegar aqui, é porque o Deadlock foi evitado!
        klog_write("[C1] Sucesso! Transferindo dados...\n");

        mutex_unlock(&arquivo_B_mutex);
        mutex_unlock(&arquivo_A_mutex);
        task_yield();
    }
}

// --- CLIENTE 2 (O da Apresentação) ---
void cliente_2() {
    while(1) {
        // PARTE 1: O Chat
        mutex_lock(&chat_mutex);
        klog_write("\n[Chat] Cliente 2: Recebendo! Vou te mandar outro.\n");
        mutex_unlock(&chat_mutex);
        delay();

        // Simulação de Deadlock por quebra da hierarquia de recursos: indução de Espera Circular (uma das Condições de Coffman).
        if (modo_seguro == 0) { 
            // ==========================================
            // APRESENTAÇÃO PARTE 1: FORÇANDO O DEADLOCK
            // ==========================================
            klog_write("[C2] (Sem Hierarquia) Trancando Arquivo B...\n");
            mutex_lock(&arquivo_B_mutex); 
            
            task_yield(); 
            
            klog_write("[C2] (Sem Hierarquia) Tentando trancar Arquivo A...\n");
            mutex_lock(&arquivo_A_mutex); // <-- VAI TRAVAR AQUI!
            
            mutex_unlock(&arquivo_A_mutex);
            mutex_unlock(&arquivo_B_mutex);

        } else {
            // ==========================================
            // APRESENTAÇÃO PARTE 2: A SOLUÇÃO
            // ==========================================
            klog_write("[C2] (Com Hierarquia) Trancando Arquivo A primeiro...\n");
            mutex_lock(&arquivo_A_mutex); 
            
            task_yield(); 
            
            klog_write("[C2] (Com Hierarquia) Trancando Arquivo B...\n");
            mutex_lock(&arquivo_B_mutex); 
            
            klog_write("[C2] Sucesso! Dados transferidos sem travar!\n");
            
            mutex_unlock(&arquivo_B_mutex);
            mutex_unlock(&arquivo_A_mutex);
        }
        
        task_yield();
    }
}

/*
 * kmain: Ponto de entrada principal do kernel, chamado pelo loader.s.
 */
int kmain(unsigned int multiboot_info_addr, 
          unsigned int kernel_virtual_start,
          unsigned int kernel_virtual_end,
          unsigned int kernel_physical_start,
          unsigned int kernel_physical_end)
{
    /* === FASE 1: Inicialização do Hardware da CPU === */
    init_gdt();
    init_idt();
    pic_remap();
    __asm__("sti");
    
    /* === FASE 2: Inicialização dos Drivers de Saída === */
    klog_init();

    klog_write("Log: Sistema Operacional Iniciado com Sucesso!\n");
    klog_write("Sistema Operacional Carregado!\n");
    
    (void)kernel_virtual_start;
    (void)kernel_virtual_end;

    klog_write("[OK] Hardware configurado.\n");
    klog_write("[OK] Paginacao ativada pelo loader.s!\n");
    klog_write("[OK] Kernel operando em 0xC0100000 (Higher Half).\n");

    /* === FASE 3: Gerenciamento de Memória (Cap. 10) === */
    struct multiboot_info *mbi = (struct multiboot_info *)multiboot_info_addr;
    pmm_init(mbi, kernel_physical_start, kernel_physical_end);
    vmm_init();
    kheap_init();

    // ========================================================
    // START DA TAREFA 05: CHAT E DEADLOCK
    // ========================================================

    // Inicializamos os Mutexes para controlar o acesso ao chat e aos arquivos
    klog_write("\n[TAREFA 05] Inicializando Mutexes...\n");
    mutex_init(&chat_mutex);
    mutex_init(&arquivo_A_mutex);
    mutex_init(&arquivo_B_mutex);
    
    // Criamos uma "thread" falsa para o kmain só para salvar o estado da pilha aqui antes de começar a trocar
    klog_write("[TAREFA 05] Criando as threads dos clientes...\n");
    create_thread(&thread_c1, cliente_1, 1);
    create_thread(&thread_c2, cliente_2, 2);

    current_thread = &thread_c1;
    next_thread = &thread_c2;

    klog_write("\n>> INICIANDO O CHAT (CUIDADO COM O DEADLOCK!) <<\n\n");
    
    // Dispara a primeira troca. O kmain vai dormir para sempre aqui.
    switch_task(&thread_kmain.esp, current_thread->esp);
    
    // Se o código chegar aqui, significa que o switch_task falhou, o que não deveria acontecer.
    klog_write("ERRO: O kernel voltou pro kmain!\n");

    /* === As Fases 4 (Módulo Externo) ficarão inalcançáveis aqui embaixo === */
    /* Isso é perfeitamente normal, pois as threads assumiram o controle da CPU */
    
    void *ptr1 = kmalloc(100);
    if (ptr1) {
        klog_write("[OK] kmalloc(100) funcionou!\n");
        kfree(ptr1);
        klog_write("[OK] kfree() funcionou!\n");
    }

        // Verifica se o GRUB carregou um módulo (nosso programa externo)
        if (!(mbi->flags & MULTIBOOT_INFO_MODS) || mbi->mods_count != 1) {
        klog_write("Erro: modulo GRUB ausente.\n");
        while (1) {
        __asm__("hlt");
        }
    }

    // O GRUB carrega o módulo para a memória, mas o kernel precisa pular para a parte virtual (Higher Half)
    struct multiboot_module *mod = (struct multiboot_module *)(mbi->mods_addr + 0xC0000000);
    klog_write("Executando modulo externo...\n");

    /* O módulo é um programa que imprime uma mensagem e volta. Foi compilado para rodar em Higher Half,
    * então já tem o 0xC0000000 incluso. */ 
    typedef void (*call_module_t)(void);
    call_module_t start_program = (call_module_t)(mod->mod_start + 0xC0000000);
    start_program();

    klog_write("Modulo retornou ao kernel.\n");
    klog_write("Pode digitar agora...\n> ");


    while(1) {
        __asm__("hlt");
    }

    return 0;
}