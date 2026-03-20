#ifndef INCLUDE_VMM_H
#define INCLUDE_VMM_H

#define PAGE_PRESENT 0x1
#define PAGE_RW      0x2
#define PAGE_USER    0x4

/* 
 * Inicializa o gerenciador de memória virtual.
 */
void vmm_init();

/* 
 * Mapeia um endereço virtual para um físico.
 */
void vmm_map(unsigned int virtual_addr, unsigned int physical_addr, unsigned int flags);

/* 
 * Desmapeia um endereço virtual.
 */
void vmm_unmap(unsigned int virtual_addr);

#endif
