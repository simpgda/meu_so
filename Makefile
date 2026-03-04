# ======================================================================
# 1. VARIÁVEIS E CONFIGURAÇÕES
# ======================================================================
# Lista de todos os arquivos objeto (.o) que compõem o nosso Sistema Operacional.
# Se criar um arquivo novo (ex: keyboard.c), basta adicionar keyboard.o aqui!
OBJECTS = loader.o kernel/kmain.o \
          io/io.o \
          drivers/fb.o drivers/serial.o \
          gdt/gdt.o gdt/gdt_s.o \
          idt/idt.o \
          interrupts/interrupts.o interrupts/interrupts_asm.o

# Compilador C (GCC) e suas flags
CC = gcc
# Flags do C: 
# -m32: Força compilação em 32-bits (o kernel x86 precisa disso).
# -nostdlib, -nostdinc, etc: Desliga a biblioteca padrão do C (não temos SO por baixo de nós para usar printf padrão, malloc, etc).
# -Wall -Wextra -Werror: Transforma todos os avisos em erros (mantém o código limpo).
# -I: Ensina o GCC onde procurar os arquivos de cabeçalho (.h) nas pastas.
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
         -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c \
         -I. -Iio -Igdt -Iidt -Iinterrupts -Idrivers

# Linker (LD) e suas flags
# -T link.ld: Usa o nosso script customizado para organizar a memória.
# -melf_i386: Formato executável ELF 32-bits.
LDFLAGS = -T link.ld -melf_i386

# Montador Assembly (NASM) e suas flags (Formato ELF)
AS = nasm
ASFLAGS = -f elf

# ======================================================================
# 2. REGRAS DE COMPILAÇÃO E LINKAGEM
# ======================================================================

# Regra padrão: O que acontece se eu digitar apenas 'make'
all: kernel.elf

# Como montar o executável final (kernel.elf):
# Ele depende de todos os arquivos listados em $(OBJECTS).
# O comando 'ld' junta todos os pedacinhos .o em um arquivo só.
kernel.elf: $(OBJECTS)
	ld $(LDFLAGS) $(OBJECTS) -o kernel.elf

# Como montar a imagem do CD (os.iso):
# Ele depende do kernel.elf estar pronto. 
# Copia o kernel pra pasta iso/boot e usa o genisoimage para empacotar tudo 
# usando o GRUB (stage2_eltorito) para tornar o CD bootável.
os.iso: kernel.elf
	cp kernel.elf iso/boot/kernel.elf
	genisoimage -R \
	-b boot/grub/stage2_eltorito \
	-no-emul-boot \
	-boot-load-size 4 \
	-A os \
	-input-charset utf8 \
	-quiet \
	-boot-info-table \
	-o os.iso \
	iso

# Como rodar o emulador:
# Depende da os.iso estar criada. Executa o Bochs com nosso arquivo de configuração.
run: os.iso
	bochs -f bochsrc.txt -q

# ======================================================================
# 3. REGRAS MÁGICAS (PATTERN RULES)
# ======================================================================
# Em vez de escrever uma regra para cada arquivo C, dizemos ao Make:
# "Para compilar qualquer .o, pegue o .c correspondente e rode o GCC"
%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

# "Para compilar qualquer .o, pegue o .s (Assembly) correspondente e rode o NASM"
%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

# ======================================================================
# 4. LIMPEZA
# ======================================================================
# Apaga todos os arquivos gerados para forçar uma recompilação limpa
clean:
	rm -rf *.o kernel.elf os.iso
	rm -rf io/*.o gdt/*.o idt/*.o interrupts/*.o drivers/*.o

# Avisa ao Make que essas palavras não são arquivos reais que ele deve procurar
.PHONY: all run clean