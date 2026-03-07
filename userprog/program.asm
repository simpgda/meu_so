; ============================================================
; program.asm - Programa Externo Carregado pelo GRUB (Capítulo 7)
;
; Este é um binário "flat" (sem formato ELF) que o GRUB carrega
; na memória como módulo. O kernel encontra o endereço dele
; via multiboot_info e faz um 'call' para cá.
;
; O que ele faz:
;   1. Coloca 0xDEADBEEF no registrador EAX (valor de teste)
;   2. Pausa o Bochs no magic breakpoint (para conferir EAX)
;   3. Retorna ao kernel com 'ret'
;
; Para verificar: no console do Bochs, após o breakpoint,
; digite 'r' e confira que EAX = DEADBEEF.
; ============================================================

bits 32                         ; Gera instruções de 32 bits
org 0                           ; Endereço base 0 (o GRUB decide onde colocar)

start:
    mov eax, 0xDEADBEEF         ; Grava um valor fácil de identificar no registrador EAX
    xchg bx, bx                 ; Magic breakpoint: o Bochs pausa a execução aqui
    ret                          ; Retorna ao kernel (volta para depois do call em kmain)
