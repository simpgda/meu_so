#ifndef INCLUDE_KHEAP_H
#define INCLUDE_KHEAP_H

#ifndef NULL
#define NULL ((void*)0)
#endif

typedef unsigned int size_t;

/* 
 * Inicializa o heap do kernel.
 */
void kheap_init();

/* 
 * Aloca memória no heap do kernel.
 */
void *kmalloc(size_t size);

/* 
 * Libera memória no heap do kernel.
 */
void kfree(void *ptr);

#endif
