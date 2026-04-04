#include "task.h"
#include "../mm/kheap.h" // kmalloc para alocar a pilha

#define STACK_SIZE 4096 // Tamanho da pilha para cada thread (4KB)
#define MAX_THREADS 8

// Variáveis globais para controle da execução atual (úteis para debug)
thread_t *current_thread;
thread_t *next_thread;

// Estruturas internas do escalonador round-robin
static thread_t *thread_table[MAX_THREADS];
static int thread_count = 0;
static int current_index = -1;

void create_thread(thread_t *thread, void (*entry_point)(), int id) {
    thread->id = id;
    // 1. Pede um bloco de memória para ser a pilha desta thread
    thread->stack_start = (unsigned int)kmalloc(STACK_SIZE);
    
    // A pilha no x86 cresce para BAIXO. Então o topo é o final do bloco.
    unsigned int *stack_top = (unsigned int *)(thread->stack_start + STACK_SIZE);

    // 2. Preparamos a pilha para enganar o 'switch_task'
    // O assembly vai dar 4 'pop's (edi, esi, ebp, ebx) e depois um 'ret'.
    // O 'ret' pula para o endereço que estiver no topo. Então colocamos a função lá!
    
    *(--stack_top) = (unsigned int)entry_point; // O endereço da função que vai rodar
    *(--stack_top) = 0; // edi falso
    *(--stack_top) = 0; // esi falso
    *(--stack_top) = 0; // ebp falso
    *(--stack_top) = 0; // ebx falso

    // 3. Salva o ESP atual na struct para quando formos pular para cá
    thread->esp = (unsigned int)stack_top;

    // 4. Registra a thread no escalonador
    if (thread_count < MAX_THREADS) {
        thread_table[thread_count] = thread;
        thread_count++;
    }

}

void task_start(thread_t *bootstrap_thread) {
    if (thread_count == 0) {
        return;
    }

    current_index = 0;
    current_thread = thread_table[current_index];
    if (thread_count > 1) {
        next_thread = thread_table[1];
    } else {
        next_thread = current_thread;
    }

    // Salva o ESP de quem chamou (ex: kmain) e entra na primeira thread
    switch_task(&bootstrap_thread->esp, current_thread->esp);
}

void task_yield(void) {
    if (thread_count <= 1 || current_index < 0) {
        return;
    }

    // Guarda a thread atual para depois voltar aqui
    thread_t *prev = thread_table[current_index];

    // Round-robin: avança para a próxima thread pronta
    current_index = (current_index + 1) % thread_count;
    current_thread = thread_table[current_index];
    next_thread = thread_table[(current_index + 1) % thread_count];

    // Chama a função em Assembly para trocar o contexto (pilha) para a próxima thread
    switch_task(&prev->esp, current_thread->esp);
}