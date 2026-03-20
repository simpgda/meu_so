#include "pmm.h"
#include "../kernel/klog.h"

#define MAX_FRAMES (1024 * 1024) // 4GB / 4KB
#define BITMAP_SIZE (MAX_FRAMES / 8)

static unsigned char bitmap[BITMAP_SIZE];

/* 
 * Marca um quadro como usado no bitmap.
 */
static void pmm_set_frame(unsigned int frame_addr) {
    unsigned int frame_index = frame_addr / PAGE_SIZE;
    unsigned int bitmap_index = frame_index / 8;
    unsigned int bit_index = frame_index % 8;
    bitmap[bitmap_index] |= (1 << bit_index);
}

/* 
 * Marca um quadro como livre no bitmap.
 */
static void pmm_clear_frame(unsigned int frame_addr) {
    unsigned int frame_index = frame_addr / PAGE_SIZE;
    unsigned int bitmap_index = frame_index / 8;
    unsigned int bit_index = frame_index % 8;
    bitmap[bitmap_index] &= ~(1 << bit_index);
}

void pmm_init(struct multiboot_info *mbi, 
              unsigned int kernel_physical_start, 
              unsigned int kernel_physical_end) {
    
    // 1. Inicializa o bitmap marcando TUDO como reservado (1)
    for (int i = 0; i < BITMAP_SIZE; i++) {
        bitmap[i] = 0xFF;
    }

    // 2. Lê o mapa de memória do multiboot para liberar RAM disponível
    if (mbi->flags & MULTIBOOT_INFO_MMAP) {
        struct multiboot_mmap_entry *mmap = (struct multiboot_mmap_entry *)(mbi->mmap_addr + 0xC0000000);
        unsigned int mmap_end = mbi->mmap_addr + mbi->mmap_length + 0xC0000000;

        while ((unsigned int)mmap < mmap_end) {
            if (mmap->type == 1) { // 1 = RAM disponível
                unsigned int start = mmap->addr_low;
                unsigned int end = start + mmap->len_low;
                
                for (unsigned int addr = start; addr < end; addr += PAGE_SIZE) {
                    if (addr < 0xFFFFF000) { // Limite de 4GB
                        pmm_clear_frame(addr);
                    }
                }
            }
            mmap = (struct multiboot_mmap_entry *)((unsigned int)mmap + mmap->size + sizeof(unsigned int));
        }
    } else {
        klog_write("Erro: Multiboot memory map não encontrado!\n");
        // Fallback usando mem_upper?
        unsigned int mem_upper_bytes = mbi->mem_upper * 1024;
        for (unsigned int addr = 0x100000; addr < (0x100000 + mem_upper_bytes); addr += PAGE_SIZE) {
            pmm_clear_frame(addr);
        }
    }

    // 3. Reserva explicitamente a memória do Kernel
    for (unsigned int addr = kernel_physical_start; addr < kernel_physical_end; addr += PAGE_SIZE) {
        pmm_set_frame(addr);
    }

    // 4. Reserva a memória dos módulos GRUB
    if (mbi->flags & MULTIBOOT_INFO_MODS) {
        struct multiboot_module *mod = (struct multiboot_module *)(mbi->mods_addr + 0xC0000000);
        for (unsigned int i = 0; i < mbi->mods_count; i++) {
            for (unsigned int addr = mod[i].mod_start; addr < mod[i].mod_end; addr += PAGE_SIZE) {
                pmm_set_frame(addr);
            }
        }
    }

    // 5. Reserva o primeiro 1MB (BIOS, GRUB, etc) para segurança
    for (unsigned int addr = 0; addr < 0x100000; addr += PAGE_SIZE) {
        pmm_set_frame(addr);
    }

    klog_write("[OK] Page Frame Allocator (PMM) inicializado.\n");
}

unsigned int pmm_alloc_frame() {
    for (unsigned int i = 0; i < BITMAP_SIZE; i++) {
        if (bitmap[i] != 0xFF) { // Se há algum bit 0 neste byte
            for (int j = 0; j < 8; j++) {
                if (!(bitmap[i] & (1 << j))) {
                    unsigned int frame_index = i * 8 + j;
                    unsigned int frame_addr = frame_index * PAGE_SIZE;
                    pmm_set_frame(frame_addr);
                    return frame_addr;
                }
            }
        }
    }
    return 0; // Out of memory
}

void pmm_free_frame(unsigned int frame_addr) {
    pmm_clear_frame(frame_addr);
}
