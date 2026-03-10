global load_idt

extern interrupt_handler

load_idt:
    mov eax, [esp + 4]
    lidt [eax]
    ret

%macro no_error_code_interrupt_handler 1
global interrupt_handler_%1
interrupt_handler_%1:
    push dword 0            ; push 0 as error code
    push dword %1           ; push the interrupt number
    jmp common_interrupt_handler
%endmacro

%macro error_code_interrupt_handler 1
global interrupt_handler_%1
interrupt_handler_%1:
    push dword %1
    jmp common_interrupt_handler
%endmacro

common_interrupt_handler:
    ; Salva todos os registradores de uso geral na pilha.
    ; A ordem aqui define a struct cpu_state no C (inversa: edi é o topo).
    ; NÃO salvamos ESP: fazer push/pop de ESP corrompe a pilha.
    push eax
    push ebx
    push ecx
    push edx
    push ebp
    push esi
    push edi

    ; Chama o handler em C (interrupts.c)
    call interrupt_handler

    ; Restaura os registradores na ordem inversa
    pop edi
    pop esi
    pop ebp
    pop edx
    pop ecx
    pop ebx
    pop eax

    ; restore the esp
    add esp, 8
    
    iret

; ====================================================================
; Exceções da CPU (0-31)
; Algumas empurram error code automaticamente, outras não.
; ====================================================================
no_error_code_interrupt_handler 0   ; #DE Division Error
no_error_code_interrupt_handler 1   ; #DB Debug
no_error_code_interrupt_handler 2   ; NMI
no_error_code_interrupt_handler 3   ; #BP Breakpoint
no_error_code_interrupt_handler 4   ; #OF Overflow
no_error_code_interrupt_handler 5   ; #BR Bound Range Exceeded
no_error_code_interrupt_handler 6   ; #UD Invalid Opcode
no_error_code_interrupt_handler 7   ; #NM Device Not Available
error_code_interrupt_handler    8   ; #DF Double Fault
no_error_code_interrupt_handler 9   ; Coprocessor Segment Overrun
error_code_interrupt_handler    10  ; #TS Invalid TSS
error_code_interrupt_handler    11  ; #NP Segment Not Present
error_code_interrupt_handler    12  ; #SS Stack-Segment Fault
error_code_interrupt_handler    13  ; #GP General Protection Fault
error_code_interrupt_handler    14  ; #PF Page Fault
no_error_code_interrupt_handler 15  ; Reserved
no_error_code_interrupt_handler 16  ; #MF x87 FPU Error
error_code_interrupt_handler    17  ; #AC Alignment Check
no_error_code_interrupt_handler 18  ; #MC Machine Check
no_error_code_interrupt_handler 19  ; #XM SIMD Exception
no_error_code_interrupt_handler 20  ; #VE Virtualization Exception
error_code_interrupt_handler    21  ; #CP Control Protection
no_error_code_interrupt_handler 22  ; Reserved
no_error_code_interrupt_handler 23  ; Reserved
no_error_code_interrupt_handler 24  ; Reserved
no_error_code_interrupt_handler 25  ; Reserved
no_error_code_interrupt_handler 26  ; Reserved
no_error_code_interrupt_handler 27  ; Reserved
no_error_code_interrupt_handler 28  ; Reserved
error_code_interrupt_handler    29  ; #HV Hypervisor Injection
error_code_interrupt_handler    30  ; #SX Security Exception
no_error_code_interrupt_handler 31  ; Reserved

; ====================================================================
; IRQs de Hardware
; ====================================================================
no_error_code_interrupt_handler 33  ; IRQ1 - Teclado
no_error_code_interrupt_handler 39  ; IRQ7 - Spurious Interrupt