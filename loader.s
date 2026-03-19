global loader                   ; Torna o rótulo 'loader' visível para o linker

; ==========================================
; CONSTANTES DO CABEÇALHO MULTIBOOT E PAGINAÇÃO
; ==========================================
MAGIC_NUMBER  equ 0x1BADB002    
ALIGN_MODULES equ 0x00000001    
CHECKSUM      equ -(MAGIC_NUMBER + ALIGN_MODULES)

KERNEL_STACK_SIZE equ 4096      ; 4 KB de pilha

; Flags da nossa Página Gigante de 4MB (Present = 1, R/W = 2, 4MB = 0x80)
PAGE_FLAGS    equ 0x00000083    

; ==========================================
; SEÇÃO BSS (Variáveis não inicializadas)
; ==========================================
section .bss
align 4096                      ; OBRIGATÓRIO: O Diretório de Páginas precisa estar alinhado em 4KB
page_directory:                 ; Array provisório de 1024 gavetas
    resb 4096

align 4                         ; Volta o alinhamento normal para a pilha
kernel_stack:                   
    resb KERNEL_STACK_SIZE      

; ==========================================
; SEÇÃO TEXT (Código Executável)
; ==========================================
section .text                   
align 4                         
    
    ; Assinatura Multiboot (GRUB)
    dd MAGIC_NUMBER             
    dd ALIGN_MODULES            
    dd CHECKSUM                 
    
extern kmain                    

; ==========================================
; FUNÇÃO LOADER (Ponto de Entrada)
; ==========================================
loader:

    ; O GRUB carrega o kernel no endereço físico 0x00100000.
    ; Como o linker script definiu a base virtual em 0xC0100000, calculamos o endereço físico do page_directory.
    ; Precisamos pegar o endereço FÍSICO do nosso page_directory para a CPU entender.
    mov eax, page_directory
    sub eax, 0xC0000000         ; eax = Endereço Físico do page_directory

    ; Configura o mapeamento de identidade (endereço físico 0x0 para virtual 0x0).
    ; Isso previne um Page Fault imediato ao ativar a paginação.
    mov dword [eax], 0x00000000 | PAGE_FLAGS

    ; 3. Mapeamento 2 (Higher-Half): Gaveta 768 aponta para 0x0 (Cobertura)
    ; 768 * 4 bytes = 3072 (que em hexadecimal é 0xC00)
    mov dword [eax + 0xC00], 0x00000000 | PAGE_FLAGS

    ; Carrega o endereço físico do diretório de páginas no registrador CR3.
    mov cr3, eax

    ; Habilita páginas de 4MB (Page Size Extension - PSE) no registrador CR4.
    mov eax, cr4
    or eax, 0x00000010
    mov cr4, eax

    ; Ativa a paginação definindo o bit 31 (PG) no registrador CR0.
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    ; Atualiza o Instruction Pointer (EIP) para o endereço virtual superior usando um salto.
    ; O registrador ECX vai receber o endereço virtual (3GB+) do rótulo 'higher_half'
    lea ecx, [higher_half]
    jmp ecx                     ; Teletransporte!

; ==========================================
;             (HIGHER HALF)
; ==========================================
higher_half:
    ; Remove o mapeamento de identidade da primeira entrada do diretório de páginas.
    ; A partir desta linha, o EIP (Ponteiro de Instrução) está rodando acima de 0xC0000000.
    
    ; Remoção do Identity Mapping da gaveta 0
    ; Agora, o endereço virtual do page_directory pode ser usado diretamente!
    mov dword [page_directory], 0
    invlpg [0]                  ; Invalida a entrada no TLB (Translation Lookaside Buffer) para o endereço 0.

    ; Configurando a Pilha (Stack):
    ; Na arquitetura x86, a pilha cresce "para baixo" (dos endereços maiores para os menores).
    ; Portanto, apontamos o ESP (Stack Pointer) para o FINAL do espaço de 4KB que reservamos.
    mov esp, kernel_stack + KERNEL_STACK_SIZE

    ; Passando informações do GRUB para o C (Cap. 7):
    ; O GRUB coloca no EBX o ponteiro físico para a struct multiboot_info,
    ; que contém dados sobre memória e módulos carregados.
    ; Adicionamos 0xC0000000 para que o kernel possa acessá-la no espaço virtual.
    add ebx, 0xC0000000
    
    ; Empurramos na pilha para virar argumento da função kmain(ebx).
    push ebx                    ; multiboot_info_addr (primeiro argumento do kmain)
    call kmain                  ; Chama a função principal do SO. O retorno (se houver) ficará no registrador EAX.                  

.loop:
    hlt                         ; Pausa a CPU até a próxima interrupção (economiza energia)
    jmp .loop