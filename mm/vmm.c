#include "vmm.h"
#include "pmm.h"
#include "../kernel/klog.h"

extern unsigned int page_directory[1024];

// Tabela de páginas para o kernel (0xC0000000 - 0xC03FFFFF)
static unsigned int kernel_page_table[1024] __attribute__((aligned(4096)));

void vmm_init() {
    // 1. Configura a tabela de páginas do kernel (0-4MB físicos -> 3GB-3GB+4MB virtuais)
    for (unsigned int i = 0; i < 1024; i++) {
        kernel_page_table[i] = (i * 4096) | PAGE_PRESENT | PAGE_RW;
    }

    // 2. Substitui a página de 4MB (PSE) por esta tabela de páginas (4KB)
    // 768 = 0xC0000000 / (4 * 1024 * 1024)
    unsigned int pte_phys = (unsigned int)kernel_page_table - 0xC0000000;
    page_directory[768] = pte_phys | PAGE_PRESENT | PAGE_RW;

    // 3. Invalida o TLB para garantir que a mudança seja aplicada
    __asm__ volatile("mov %0, %%cr3" : : "r"((unsigned int)page_directory - 0xC0000000));

    klog_write("[OK] Virtual Memory Manager (VMM) inicializado.\n");
}

/* 
 * Endereço virtual reservado para mapeamentos temporários (Cap. 10.2).
 * Como mapeamos a kernel_page_table em 0xC0000000, e ela tem 1024 entradas,
 * a última entrada (1023) corresponde ao endereço virtual 0xC03FF000.
 */
#define TEMP_MAP_ADDR 0xC03FF000

void vmm_map(unsigned int virtual_addr, unsigned int physical_addr, unsigned int flags) {
    unsigned int pd_idx = virtual_addr >> 22;
    unsigned int pt_idx = (virtual_addr >> 12) & 0x3FF;

    if (!(page_directory[pd_idx] & PAGE_PRESENT)) {
        // Precisamos criar uma nova tabela de páginas
        unsigned int pt_phys = pmm_alloc_frame();
        
        // Mapeia temporariamente para zerar a nova PT
        kernel_page_table[1023] = pt_phys | PAGE_PRESENT | PAGE_RW;
        __asm__ volatile("invlpg (%0)" : : "r"(TEMP_MAP_ADDR));
        
        unsigned int *pt_virt = (unsigned int *)TEMP_MAP_ADDR;
        for (int i = 0; i < 1024; i++) pt_virt[i] = 0;
        
        page_directory[pd_idx] = pt_phys | PAGE_PRESENT | PAGE_RW | flags;
    }

    // Agora pegamos a PT (pode ser a do kernel ou uma nova)
    // Se for a do kernel (768), usamos diretamente kernel_page_table
    if (pd_idx == 768) {
        kernel_page_table[pt_idx] = physical_addr | PAGE_PRESENT | flags;
    } else {
        // Para outras, mapeamos a PT temporariamente para atualizar
        unsigned int pt_phys = page_directory[pd_idx] & ~0xFFF;
        kernel_page_table[1023] = pt_phys | PAGE_PRESENT | PAGE_RW;
        __asm__ volatile("invlpg (%0)" : : "r"(TEMP_MAP_ADDR));
        
        unsigned int *pt_virt = (unsigned int *)TEMP_MAP_ADDR;
        pt_virt[pt_idx] = physical_addr | PAGE_PRESENT | flags;
    }

    __asm__ volatile("invlpg (%0)" : : "r"(virtual_addr));
}

void vmm_unmap(unsigned int virtual_addr) {
    unsigned int pd_idx = virtual_addr >> 22;
    unsigned int pt_idx = (virtual_addr >> 12) & 0x3FF;

    if (!(page_directory[pd_idx] & PAGE_PRESENT)) return;

    if (pd_idx == 768) {
        kernel_page_table[pt_idx] = 0;
    } else {
        unsigned int pt_phys = page_directory[pd_idx] & ~0xFFF;
        kernel_page_table[1023] = pt_phys | PAGE_PRESENT | PAGE_RW;
        __asm__ volatile("invlpg (%0)" : : "r"(TEMP_MAP_ADDR));
        
        unsigned int *pt_virt = (unsigned int *)TEMP_MAP_ADDR;
        pt_virt[pt_idx] = 0;
    }
    __asm__ volatile("invlpg (%0)" : : "r"(virtual_addr));
}
