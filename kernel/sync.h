#ifndef SYNC_H
#define SYNC_H

// Estrutura para uma trava simples (MUTEX)
typedef struct {
    int locked; // 0 = destravado, 1 = travado
} mutex_t;

// Funções para manipular o MUTEX
void mutex_init(mutex_t *m);
void mutex_lock(mutex_t *m);
void mutex_unlock(mutex_t *m);

#endif