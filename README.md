# Sistemas Operacionais

Projeto de desenvolvimento de um Kernel simples (x86), implementando bootloader, GDT, IDT e tratamento de interrupções básicas.

## Alunos

- Gabriel de Albuquerque Simplício
- João Pedro Lima Dantas
- João Henrique Alves de Sousa
- Lucas Franca de Melo Batista

## Como Executar
O projeto utiliza `Make` para automatizar a compilação e execução.

Deve ter instalado: `build-essential` / `nasm` / `genisoimage` / `bochs` e `bochs-x`
Para compilar todo o projeto, gerar a ISO e abrir o emulador automaticamente, rode:

```
```bash
make run
```

## Entrega parcial (11/02) - CAP: 02, 03
- Booting: Configuração do cabeçalho Multiboot e inicialização do Kernel via GRUB.
- Transição para C: Configuração da pilha (stack) e chamada da função principal kmain em C.

## Entrega parcial (02/03) - CAP: 04, 05, 06
- Output: Implementação de drivers para Framebuffer (saída de vídeo VGA) e Porta Serial (COM1 para logs).
- GDT (Global Descriptor Table): Configuração da segmentação de memória para permitir o acesso aos 4GB de endereçamento em modo protegido.
- IDT & Interrupções: Criação da Interrupt Descriptor Table e configuração do PIC para capturar eventos de hardware, como o Driver de Teclado.
