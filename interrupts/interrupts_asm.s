; Expõe as funções para que o código em C (kmain.c ou idt.c) possa enxergá-las e chamá-las
global load_idt
global interrupt_handler_33

; Importa a função principal de tratamento escrita em C (que está no seu interrupts.c)
extern interrupt_handler

; =====================================================================
; CARREGAMENTO DA IDT (Interrupt Descriptor Table)
; =====================================================================
; Assinatura no C: void load_idt(unsigned int idt_ptr);
load_idt:
    mov eax, [esp + 4]  ; Pega o 1º argumento passado pelo C (o ponteiro para a struct da IDT)
    lidt [eax]          ; Carrega a IDT no processador usando a instrução especial 'lidt'
    ret                 ; Retorna a execução para a função C que a chamou



; =====================================================================
; MACROS PARA GERAR HANDLERS DE INTERRUPÇÃO
; =====================================================================

; Macro para interrupções em que a CPU NÃO gera código de erro automaticamente 
; (Isso vale para interrupções de hardware, como teclado e timer)
%macro no_error_code_interrupt_handler 1
global interrupt_handler_%1
interrupt_handler_%1:
    push dword 0            ; Empilha um 0 como "código de erro falso" para padronizar a pilha
    push dword %1           ; Empilha o número da interrupção (passado no %1)
    jmp common_interrupt_handler ; Pula para o tratador comum lá embaixo
%endmacro

; Macro para interrupções em que a CPU JÁ GERA um código de erro 
; (Geralmente exceções da CPU, como Page Fault ou General Protection Fault)
%macro error_code_interrupt_handler 1
global interrupt_handler_%1
interrupt_handler_%1:
    ; A CPU já empilhou o código de erro sozinha antes de chegar nesta linha
    push dword %1           ; Empilha apenas o número da interrupção
    jmp common_interrupt_handler ; Pula para o tratador comum
%endmacro

; =====================================================================
; TRATADOR COMUM DE INTERRUPÇÕES (O funil por onde todas passam)
; =====================================================================
common_interrupt_handler:
    ; 1. Salva o contexto atual (todos os registradores de uso geral).
    ; A ordem aqui define a "struct cpu_state" lá no seu C.
    ; NÃO salvamos o ESP aqui porque usar push/pop no ESP corromperia a própria pilha.
    push eax
    push ebx
    push ecx
    push edx
    push ebp
    push esi
    push edi

    ; 2. Chama o handler em C (a função interrupt_handler em interrupts.c)
    ; O C vai ler essa pilha toda para saber o que aconteceu e qual interrupção foi disparada.
    call interrupt_handler

    ; 3. Restaura os registradores na ordem inversa EXATA em que foram salvos
    pop edi
    pop esi
    pop ebp
    pop edx
    pop ecx
    pop ebx
    pop eax

    ; 4. Limpa a pilha. Removemos o número da interrupção e o código de erro 
    ; que as macros empilharam lá em cima (2 pushes de 4 bytes = 8 bytes adicionados ao ESP).
    add esp, 8
    
    ; 5. Retorna da interrupção. A instrução 'iret' é sagrada: ela diz à CPU 
    ; para restaurar o EIP, o CS e as EFLAGS originais de antes da interrupção acontecer.
    iret

; =====================================================================
; INSTANCIAÇÃO DOS HANDLERS
; =====================================================================
; Cria a função interrupt_handler_33 usando a macro "sem código de erro".
; O número 33 (IRQ1) é a interrupção que o hardware do Teclado (PS/2) dispara.
no_error_code_interrupt_handler 33

; =====================================================================
; MOTOR DE TROCA DE CONTEXTO (Multitarefa Cooperativa - Cap 14)
; =====================================================================

global switch_task

; Assinatura no C: void switch_task(unsigned int *old_esp_ptr, unsigned int new_esp);
switch_task:
    ; 1. Salva os registradores de uso geral que o C exige preservar (callee-saved)
    push ebx
    push ebp
    push esi
    push edi

    ; 2. Pega os argumentos passados pelo C. 
    ; Como fizemos 4 'push' (16 bytes) e há o endereço de retorno do 'call' (+4 bytes), 
    ; precisamos somar 20 e 24 ao ESP para acessar os argumentos originais.
    mov eax, [esp + 20] ; eax = primeiro argumento (ponteiro para salvar o ESP atual)
    mov edx, [esp + 24] ; edx = segundo argumento (valor do novo ESP a ser carregado)

    ; 3. Salva o topo da pilha atual no endereço apontado por eax (ex: &current->esp)
    mov [eax], esp

    ; =================================================================
    ; 4. A TROCA REAL: Substitui a pilha da CPU pela pilha da nova thread
    ; =================================================================
    mov esp, edx

    ; 5. Restaura os registradores (os valores agora estão saindo da NOVA pilha)
    pop edi
    pop esi
    pop ebp
    pop ebx

    ; 6. Retorna. O 'ret' desempilha o endereço de retorno da nova pilha, 
    ; fazendo a CPU continuar a execução exatamente onde a nova thread havia parado!
    ret