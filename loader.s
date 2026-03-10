global loader
extern kmain

MAGIC_NUMBER equ 0x1BADB002
FLAGS equ 0x0
CHECKSUM equ -MAGIC_NUMBER
KERNEL_STACK_SIZE equ 4096

section .text
align 4
    dd MAGIC_NUMBER
    dd FLAGS
    dd CHECKSUM

loader:
    mov esp, kernel_stack + KERNEL_STACK_SIZE

    ; Configura uma IDT mínima para que exceções não causem triple fault
    call setup_idt

    call kmain

.loop:
    hlt
    jmp .loop

; ==========================================
; IDT MÍNIMA - captura exceções ao invés de triple fault
; ==========================================
setup_idt:
    ; Preenche todas as 256 entradas da IDT com o handler genérico
    mov edi, idt_table
    mov ecx, 256
.fill_idt:
    mov eax, default_fault_handler
    mov word [edi],     ax          ; offset_low
    mov word [edi + 2], 0x08       ; seletor do code segment (GRUB configura CS=0x08)
    mov byte [edi + 4], 0          ; zero
    mov byte [edi + 5], 0x8E       ; type: 32-bit interrupt gate, DPL=0, present
    shr eax, 16
    mov word [edi + 6], ax          ; offset_high
    add edi, 8
    dec ecx
    jnz .fill_idt

    ; Carrega a IDT
    lidt [idt_pointer]
    ret

; Handler que imprime "FAULT!" no VGA e trava (em vez de triple fault silencioso)
default_fault_handler:
    ; Escreve "FAULT!" na tela VGA para debugar
    mov dword [0xB8000],  0x4C464C46   ; "FF" (vermelho em branco) - ajusta para "FA"
    mov dword [0xB8004],  0x4C554C41   ; "AU"
    mov dword [0xB8008],  0x4C544C4C   ; "LT"
    mov dword [0xB800C],  0x4C214C21   ; "!!"
    cli
.halt:
    hlt
    jmp .halt

section .data
align 4
idt_pointer:
    dw (256 * 8) - 1        ; limit: 256 entradas * 8 bytes - 1
    dd idt_table             ; base: endereço da tabela

section .bss
align 4
kernel_stack:
    resb KERNEL_STACK_SIZE
idt_table:
    resb 256 * 8             ; 256 entradas de 8 bytes cada