OBJECTS = loader.o kmain.o \
          io/io.o io/io_func.o \
          gdt/gdt.o gdt/gdt_asm.o \
          idt/idt.o \
          interrupts/interrupts.o interrupts/interrupts_asm.o

CC = gcc

# Adicionados -fno-pie e -fno-pic aqui:
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
         -nostartfiles -nodefaultlibs -fno-pie -fno-pic -Wall -Wextra -Werror -c \
         -I. -Iio -Igdt -Iidt -Iinterrupts

# Adicionado -no-pie no linker:
LDFLAGS = -T link.ld -melf_i386 -no-pie

AS = nasm
ASFLAGS = -f elf

all: kernel.elf

kernel.elf: $(OBJECTS)
	ld $(LDFLAGS) $(OBJECTS) -o kernel.elf

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

run: os.iso
	bochs -f bochsrc.txt -q

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -rf *.o kernel.elf os.iso
	rm -rf io/*.o gdt/*.o idt/*.o interrupts/*.o