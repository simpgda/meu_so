#ifndef TASK_H
#define TASK_H

// A estrutura que guarda o estado de cada "cliente" ou "servidor"
typedef struct {
    unsigned int esp;         // Onde a pilha parou (usado pelo switch_task)
    unsigned int stack_start; // Início da memória alocada (para o kmalloc/kfree)
    int id;                   // ID da thread (ex: 0=Servidor, 1=Cliente1...)
} thread_t;

// A função em Assembly que você acabou de compilar!
extern void switch_task(unsigned int *old_esp_ptr, unsigned int new_esp);

// A função C que vai "gerar" uma thread nova
void create_thread(thread_t *thread, void (*entry_point)(), int id);

// Variáveis globais para controlar qual thread está rodando e qual é a próxima
extern thread_t *current_thread;
extern thread_t *next_thread;
void task_yield(void);

#endif
