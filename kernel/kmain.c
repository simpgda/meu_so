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

// Primitivas de sincronização para garantir Exclusão Mútua em log, fila do chat e arquivos.
mutex_t chat_log_mutex;
mutex_t chat_queue_mutex;
mutex_t arquivo_A_mutex;  
mutex_t arquivo_B_mutex;  

#define CHAT_MAX_MSGS 8
#define CHAT_MSG_LEN 80

typedef struct {
    int from_id;
    char text[CHAT_MSG_LEN];
} chat_msg_t;

chat_msg_t chat_queue[CHAT_MAX_MSGS];
int chat_head = 0;
int chat_tail = 0;
int chat_count = 0;

// Threads do servidor/chat e dos clientes
thread_t thread_server;
thread_t thread_c1;
thread_t thread_c2;
thread_t thread_c3;
thread_t thread_kmain; 

static void delay() {
    for (volatile unsigned int i = 0; i < 9000000; i++);
}

static void str_copy(char *dst, const char *src, unsigned int max_len) {
    unsigned int i = 0;

    if (max_len == 0) {
        return;
    }

    while (i + 1 < max_len && src[i] != '\0') {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

static int chat_send(int from_id, const char *text) {
    int ok = 0;

    mutex_lock(&chat_queue_mutex);
    if (chat_count < CHAT_MAX_MSGS) {
        chat_queue[chat_tail].from_id = from_id;
        str_copy(chat_queue[chat_tail].text, text, CHAT_MSG_LEN);
        chat_tail = (chat_tail + 1) % CHAT_MAX_MSGS;
        chat_count++;
        ok = 1;
    }
    mutex_unlock(&chat_queue_mutex);

    return ok;
}

static int chat_receive(chat_msg_t *out_msg) {
    int ok = 0;

    mutex_lock(&chat_queue_mutex);
    if (chat_count > 0) {
        *out_msg = chat_queue[chat_head];
        chat_head = (chat_head + 1) % CHAT_MAX_MSGS;
        chat_count--;
        ok = 1;
    }
    mutex_unlock(&chat_queue_mutex);

    return ok;
}

static void log_server_message(const chat_msg_t *msg) {
    mutex_lock(&chat_log_mutex);
    if (msg->from_id == 1) {
        klog_write("[Servidor] Cliente 1: ");
    } else if (msg->from_id == 2) {
        klog_write("[Servidor] Cliente 2: ");
    } else if (msg->from_id == 3) {
        klog_write("[Servidor] Cliente 3: ");
    } else {
        klog_write("[Servidor] Cliente ?: ");
    }
    klog_write(msg->text);
    klog_write("\n");
    mutex_unlock(&chat_log_mutex);
}

void servidor_chat() {
    while (1) {
        chat_msg_t msg;

        if (chat_receive(&msg)) {
            log_server_message(&msg);
        } else {
            task_yield();
        }
    }
}

// --- CLIENTE 1 (O Padrão) ---
void cliente_1() {
    while (1) {
        while (!chat_send(1, "Te enviei um arquivo, Cliente 2!")) {
            task_yield();
        }
        delay();

        mutex_lock(&chat_log_mutex);
        klog_write("[C1] Trancando Arquivo A...\n");
        mutex_unlock(&chat_log_mutex);
        mutex_lock(&arquivo_A_mutex);
        
        task_yield();
        
        mutex_lock(&chat_log_mutex);
        klog_write("[C1] Tentando trancar Arquivo B...\n");
        mutex_unlock(&chat_log_mutex);
        mutex_lock(&arquivo_B_mutex); 
        
        mutex_lock(&chat_log_mutex);
        klog_write("[C1] Sucesso! Transferindo dados...\n");
        mutex_unlock(&chat_log_mutex);

        mutex_unlock(&arquivo_B_mutex);
        mutex_unlock(&arquivo_A_mutex);

        while (!chat_send(1, "Transferencia finalizada.")) {
            task_yield();
        }

        delay();
        task_yield();
    }
}

// --- CLIENTE 2 (O da Apresentação) ---
void cliente_2() {
    while (1) {
        while (!chat_send(2, "Recebendo! Vou te mandar outro.")) {
            task_yield();
        }
        delay();

        // Simulação de Deadlock por quebra da hierarquia de recursos: indução de Espera Circular (uma das Condições de Coffman).
        if (modo_seguro == 0) { 
            // ==========================================
            // APRESENTAÇÃO PARTE 1: FORÇANDO O DEADLOCK
            // ==========================================
            mutex_lock(&chat_log_mutex);
            klog_write("[C2] (Sem Hierarquia) Trancando Arquivo B...\n");
            mutex_unlock(&chat_log_mutex);
            mutex_lock(&arquivo_B_mutex); 
            
            task_yield(); 
            
            mutex_lock(&chat_log_mutex);
            klog_write("[C2] (Sem Hierarquia) Tentando trancar Arquivo A...\n");
            mutex_unlock(&chat_log_mutex);
            mutex_lock(&arquivo_A_mutex); // <-- VAI TRAVAR AQUI!
            
            mutex_unlock(&arquivo_A_mutex);
            mutex_unlock(&arquivo_B_mutex);

        } else {
            // ==========================================
            // APRESENTAÇÃO PARTE 2: A SOLUÇÃO
            // ==========================================
            mutex_lock(&chat_log_mutex);
            klog_write("[C2] (Com Hierarquia) Trancando Arquivo A primeiro...\n");
            mutex_unlock(&chat_log_mutex);
            mutex_lock(&arquivo_A_mutex); 
            
            task_yield(); 
            
            mutex_lock(&chat_log_mutex);
            klog_write("[C2] (Com Hierarquia) Trancando Arquivo B...\n");
            mutex_unlock(&chat_log_mutex);
            mutex_lock(&arquivo_B_mutex); 
            
            mutex_lock(&chat_log_mutex);
            klog_write("[C2] Sucesso! Dados transferidos sem travar!\n");
            mutex_unlock(&chat_log_mutex);
            
            mutex_unlock(&arquivo_B_mutex);
            mutex_unlock(&arquivo_A_mutex);

            while (!chat_send(2, "Transferencia sem deadlock (hierarquia OK).")) {
                task_yield();
            }
        }

        delay();
        task_yield();
    }
}

void cliente_3() {
    while (1) {
        while (!chat_send(3, "Servidor, confirme se meu ping chegou.")) {
            task_yield();
        }
        delay();
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
    mutex_init(&chat_log_mutex);
    mutex_init(&chat_queue_mutex);
    mutex_init(&arquivo_A_mutex);
    mutex_init(&arquivo_B_mutex);
    
    // Criamos as threads do servidor e dos clientes
    klog_write("[TAREFA 05] Criando thread do servidor e clientes...\n");
    create_thread(&thread_server, servidor_chat, 0);
    create_thread(&thread_c1, cliente_1, 1);
    create_thread(&thread_c2, cliente_2, 2);
    create_thread(&thread_c3, cliente_3, 3);

    if (modo_seguro == 0) {
        klog_write("\n>> MODO DEADLOCK: C1 e C2 podem travar recursos A/B <<\n");
    } else {
        klog_write("\n>> MODO SEGURO: hierarquia de travas evita deadlock <<\n");
    }
    klog_write(">> INICIANDO CHAT MULTI-USUARIO COM SERVIDOR <<\n\n");
    
    // Inicia o escalonador a partir do contexto do kmain
    task_start(&thread_kmain);
    
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