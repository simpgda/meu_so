#include "task.h"
#include "../mm/kheap.h" // kmalloc para alocar a pilha

#define STACK_SIZE 4096 // Tamanho da pilha para cada thread (4KB)

    // Variáveis globais para controlar qual thread está rodando e qual é a próxima
    thread_t *current_thread;
    thread_t *next_thread;

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

}

void task_yield(void) {
    // Guarda a thread atual para depois voltar aqui
    thread_t *prev = current_thread;
    
    // Decide qual é a próxima thread (aqui só alternamos entre 2 threads, mas poderia ser uma fila) (Pig-Pong)
    current_thread = next_thread;
    next_thread = prev;
    
    // Chama a função em Assembly para trocar o contexto (pilha) para a próxima thread
    switch_task(&prev->esp, current_thread->esp);
}