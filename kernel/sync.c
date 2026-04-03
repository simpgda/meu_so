#include "sync.h"
#include "task.h"

void mutex_init(mutex_t *m) {
    m->locked = 0; // Começa destrancado
}

void mutex_lock(mutex_t *m) {
    // Enquanto estiver trancado por outra thread...
    while (m->locked == 1) {
        task_yield(); // ...passa a vez para a próxima thread rodar!
    }
    // Quando sair do while, significa que destrancaram. Nós trancamos agora!
    m->locked = 1;
}

void mutex_unlock(mutex_t *m) {
    m->locked = 0; // Libera a chave
}