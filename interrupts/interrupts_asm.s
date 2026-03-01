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
    ; save the registers
    push eax
    push ebx
    push ecx
    push edx
    push ebp
    push esp
    push esi
    push edi

    ; call the C function
    call interrupt_handler

    ; restore the registers
    pop edi
    pop esi
    pop esp
    pop ebp
    pop edx
    pop ecx
    pop ebx
    pop eax

    ; restore the esp
    add esp, 8
    
    iret

no_error_code_interrupt_handler 33