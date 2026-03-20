#ifndef INCLUDE_PMM_H
#define INCLUDE_PMM_H

#include "../kernel/multiboot.h"

#define PAGE_SIZE 4096

/* 
 * Inicializa o alocador de quadros de página (PMM).
 * @param mbi Ponteiro para a struct multiboot_info.
 * @param kernel_physical_start Início físico do kernel.
 * @param kernel_physical_end Fim físico do kernel.
 */
void pmm_init(struct multiboot_info *mbi, 
              unsigned int kernel_physical_start, 
              unsigned int kernel_physical_end);

/* 
 * Aloca um quadro de página físico.
 * @return Endereço físico do quadro alocado, ou 0 se falhar.
 */
unsigned int pmm_alloc_frame();

/* 
 * Libera um quadro de página físico.
 * @param frame_addr Endereço físico do quadro a liberar.
 */
void pmm_free_frame(unsigned int frame_addr);

#endif
