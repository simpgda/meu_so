#include "kheap.h"
#include "pmm.h"
#include "vmm.h"
#include "../kernel/klog.h"

typedef long Align;

union header {
    struct {
        union header *ptr;
        unsigned int size;
    } s;
    Align x;
};

typedef union header Header;

static Header base;
static Header *freep = NULL;

#define HEAP_START 0xC0400000
static unsigned int heap_end = HEAP_START;

static Header *morecore(unsigned int nu) {
    unsigned int nbytes;
    void *p;

    if (nu < 1024) nu = 1024; // Pelo menos 1024 unidades
    nbytes = nu * sizeof(Header);

    // Aloca quadros de página suficientes
    unsigned int pages_needed = (nbytes + PAGE_SIZE - 1) / PAGE_SIZE;
    for (unsigned int i = 0; i < pages_needed; i++) {
        unsigned int phys = pmm_alloc_frame();
        if (phys == 0) return NULL;
        vmm_map(heap_end, phys, PAGE_RW);
        heap_end += PAGE_SIZE;
    }

    p = (void *)(heap_end - pages_needed * PAGE_SIZE);
    Header *up = (Header *)p;
    up->s.size = (pages_needed * PAGE_SIZE) / sizeof(Header);
    kfree((void *)(up + 1));
    return freep;
}

void kheap_init() {
    freep = NULL;
    klog_write("[OK] Kernel Heap inicializado.\n");
}

void *kmalloc(size_t nbytes) {
    Header *p, *prevp;
    unsigned int nunits;

    nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;
    if ((prevp = freep) == NULL) {
        base.s.ptr = freep = prevp = &base;
        base.s.size = 0;
    }

    for (p = prevp->s.ptr; ; prevp = p, p = p->s.ptr) {
        if (p->s.size >= nunits) {
            if (p->s.size == nunits) {
                prevp->s.ptr = p->s.ptr;
            } else {
                p->s.size -= nunits;
                p += p->s.size;
                p->s.size = nunits;
            }
            freep = prevp;
            return (void *)(p + 1);
        }
        if (p == freep) {
            if ((p = morecore(nunits)) == NULL) return NULL;
        }
    }
}

void kfree(void *ap) {
    Header *bp, *p;

    bp = (Header *)ap - 1;
    for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr) {
        if (p >= p->s.ptr && (bp > p || bp < p->s.ptr)) break;
    }

    if (bp + bp->s.size == p->s.ptr) {
        bp->s.size += p->s.ptr->s.size;
        bp->s.ptr = p->s.ptr->s.ptr;
    } else {
        bp->s.ptr = p->s.ptr;
    }

    if (p + p->s.size == bp) {
        p->s.size += bp->s.size;
        p->s.ptr = bp->s.ptr;
    } else {
        p->s.ptr = bp;
    }
    freep = p;
}
