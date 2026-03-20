# ======================================================================
# 1. VARIÁVEIS E CONFIGURAÇÕES
# ======================================================================
# Lista de todos os arquivos objeto (.o) que compõem o nosso Sistema Operacional.
OBJECTS = loader.o kernel/kmain.o kernel/klog.o \
          io/io.o \
          drivers/fb.o drivers/serial.o \
          gdt/gdt.o gdt/gdt_s.o \
          idt/idt.o \
          interrupts/interrupts.o interrupts/interrupts_asm.o \
          interrupts/pic.o \
          mm/pmm.o mm/vmm.o mm/kheap.o

# Compilador C (GCC) e suas flags
CC = gcc
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
         -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c \
         -I. -Iio -Igdt -Iidt -Iinterrupts -Idrivers -Ikernel -Imm

# Linker (LD) e suas flags
LDFLAGS = -T link.ld -melf_i386

# Montador Assembly (NASM) e suas flags (Formato ELF)
AS = nasm
ASFLAGS = -f elf

# ======================================================================
# 2. REGRAS DE COMPILAÇÃO E LINKAGEM
# ======================================================================

# Regra padrão
all: kernel.elf

# Como montar o executável final (kernel.elf):
kernel.elf: $(OBJECTS)
	ld $(LDFLAGS) $(OBJECTS) -o kernel.elf

# Como montar a imagem do CD (os.iso):
os.iso: kernel.elf iso/modules/program
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
run: os.iso
	bochs -f bochsrc.txt -q

# ======================================================================
# 3. REGRAS MÁGICAS (PATTERN RULES)
# ======================================================================
%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

# ======================================================================
# 4. LIMPEZA
# ======================================================================
# Apaga todos os arquivos gerados para forçar uma recompilação limpa
clean:
	rm -rf *.o kernel.elf os.iso
	rm -rf io/*.o gdt/*.o idt/*.o interrupts/*.o drivers/*.o kernel/*.o mm/*.o
	rm -rf iso/modules/program

# Compila o programa externo como binário flat
iso/modules/program: userprog/program.asm
	mkdir -p iso/modules
	$(AS) -f bin $< -o $@

.PHONY: all run clean