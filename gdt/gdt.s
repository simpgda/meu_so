global gdt_flush

gdt_flush:
    ; Recebe como parâmetro o ponteiro para a estrutura GDTR
    ; (formato: limite de 16 bits + base de 32 bits).
    ; Convenção cdecl: primeiro argumento está em [esp + 4].
    mov eax, [esp + 4]

    ; Carrega o registrador GDTR com o novo endereço da GDT.
    ; A partir daqui, a CPU passa a usar a nova tabela de descritores.
    lgdt [eax]

    ; Após trocar a GDT, é necessário recarregar todos os registradores
    ; de segmento para que utilizem os novos descritores.
    ; 0x10 corresponde ao seletor do segmento de dados (índice 2).
    mov ax, 0x10
    mov ds, ax          ; Data Segment
    mov es, ax          ; Extra Segment
    mov fs, ax          ; Extra Segment
    mov gs, ax          ; Extra Segment
    mov ss, ax          ; Stack Segment

    ; O registrador CS não pode ser alterado diretamente.
    ; Um salto longo (far jump) força o processador a recarregar
    ; o CS com o seletor 0x08 (segmento de código, índice 1).
    jmp 0x08:.flush

.flush:
    ; Retorna para o chamador já com a nova GDT ativa
    ; e todos os segmentos devidamente atualizados.
    ret