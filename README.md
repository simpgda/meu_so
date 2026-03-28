# 💻 Sistemas Operacionais - Kernel Simples (x86)

Este projeto consiste no desenvolvimento de um kernel monolítico simples para a arquitetura x86, abordando conceitos fundamentais de sistemas operacionais como bootloader, segmentação, gerenciamento de interrupções, memória e execução de programas externos.

---

## 👥 Alunos
- **Gabriel de Albuquerque Simplício**
- **João Pedro Lima Dantas**
- **João Henrique Alves de Sousa**
- **Lucas Franca de Melo Batista**

---

## 🚀 Como Executar

### Pré-requisitos
O projeto utiliza `Make` para automatizar a compilação e execução. Certifique-se de ter instalado:
- `build-essential`
- `nasm`
- `genisoimage`
- `bochs` e `bochs-x`

### Linux (Ubuntu/WSL2)
Para compilar todo o projeto, gerar a imagem ISO e abrir o emulador automaticamente:
```bash
make run
```

---

## 📅 Histórico de Entregas

### ✅ Entrega Parcial (11/02) - Booting & C
- **Booting:** Configuração do cabeçalho Multiboot e inicialização do Kernel via GRUB.
- **Transição para C:** Configuração da pilha (stack) em Assembly e chamada da função principal `kmain` em C.

### ✅ Entrega Parcial (02/03) - I/O & CPU Control
- **Output:** Implementação de drivers para **Framebuffer** (saída VGA) e **Porta Serial** (logs em COM1).
- **GDT (Global Descriptor Table):** Configuração da segmentação de memória para permitir o acesso aos 4GB de endereçamento.
- **IDT & Interrupções:** Criação da Interrupt Descriptor Table e configuração do PIC para capturar eventos de hardware (teclado e timers).

### ✅ Entrega Parcial (11/03) - Módulos & User Programs (CAP 07)
- **Módulos GRUB:** Suporte para carregamento de programas externos via comando `module` no `menu.lst`.
- **Multiboot Integration:** Repasse das informações de módulos (`mods_addr` e `mods_count`) para o kernel.
- **Execução de Binários:** Carregamento de binários flat e salto para execução a partir do kernel.

### ✅ Entrega Parcial (23/03) - Gerenciamento de Memória (CAP 08, 09, 10)
- **PMM (Physical Memory Manager):** Alocador de quadros físicos baseado em **Bitmap**, gerenciando a memória RAM disponível informada pelo Multiboot.
- **VMM & Paging (CAP 09):** Implementação de paginação de dois níveis. Configuração do **Higher Half Kernel** (mapeado em 0xC0000000) e funções de `map`/`unmap` de endereços virtuais.
- **Kernel Heap (CAP 10):** Implementação de alocador de memória dinâmica para o kernel (`kmalloc` e `kfree`), permitindo estruturas de dados flexíveis.
- **User Mode Foundation (CAP 08):** Estrutura inicial para suporte a Ring 3 e isolamento de processos.

---

## 📂 Estrutura do Projeto
```text
.
├── drivers/        # Drivers de hardware (Framebuffer, Serial, Teclado)
├── gdt/            # Configuração da Global Descriptor Table
├── idt/            # Configuração da Interrupt Descriptor Table
├── interrupts/     # Handlers de interrupções (ISRs e IRQs)
├── io/             # Funções de entrada/saída de baixo nível (Assembly)
├── iso/            # Arquivos para criação da imagem ISO (GRUB/menu.lst)
├── kernel/         # Código principal (kmain, logs e inicialização)
├── mm/             # Gerenciamento de Memória (PMM, VMM, Heap)
├── userprog/       # Programas de usuário (módulos externos)
├── loader.s        # Ponto de entrada em Assembly (Multiboot header)
└── link.ld         # Linker script para organizar as seções do kernel
```

---

## 🏗️ Detalhes de Arquitetura

### Higher Half Kernel
O kernel é carregado fisicamente em `0x100000` (1MB), mas mapeado virtualmente em `0xC0100000` (3GB+1MB). Isso permite que o espaço de endereçamento do usuário comece em endereços baixos, mantendo o kernel protegido no topo da memória.

### Gerenciamento de Memória
- **PMM:** Utiliza um **Bitmap** para rastrear quadros físicos de 4KB.
- **VMM:** Implementa paginação de dois níveis (*Page Directory* e *Page Tables*).
- **Heap:** Alocador simples para memória dinâmica dentro do kernel.

---

## 🐞 Depuração (Debugging)

Para debugar o kernel usando as ferramentas integradas do Bochs:

1. Execute o comando: `make debug`
2. O Bochs iniciará pausado. No terminal, você pode usar comandos como:
   - `c`: Continua a execução.
   - `s`: Executa a próxima instrução (step).
   - `r`: Mostra o estado dos registradores (EAX, EBX, ESP, etc).
   - `info cpu`: Mostra informações detalhadas da CPU.            
   - `xp /n addr`: Examina o conteúdo da memória física.

---

## 🛠️ Ferramentas Utilizadas
- **NASM:** Assembler para o bootloader e rotinas de baixo nível.
- **GCC (i686-elf):** Compilador C para o kernel.
- **Bochs:** Emulador x86 com debugger integrado.
- **Makefile:** Orquestração de build.
