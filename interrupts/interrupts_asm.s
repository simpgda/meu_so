global load_idt
global interrupt_handler_33

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

no_error_code_interrupt_handler 33