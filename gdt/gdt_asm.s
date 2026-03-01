global load_gdt

; load_gdt - Loads the GDT descriptor table.
; stack: [esp + 4] the address of the struct gdt
load_gdt:
    mov eax, [esp + 4]  ; Get the pointer to the GDT descriptor
    lgdt [eax]          ; Load the GDT

    ; Reload data segment registers
    mov ax, 0x10
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Reload cs register using a far jump
    jmp 0x08:.flush_cs

.flush_cs:
    ret