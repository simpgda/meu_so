#include "idt.h"
#include "pic.h"
#include "klog.h"
#include "io.h"

/* Nomes das exceções da CPU para mensagens de erro */
static const char *exception_names[] = {
    "Division Error",
    "Debug",
    "NMI",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 FPU Error",
    "Alignment Check",
    "Machine Check",
    "SIMD Exception",
    "Virtualization Exception",
    "Control Protection",
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved",
    "Hypervisor Injection",
    "Security Exception",
    "Reserved"
};

/* ========================================================================= *
 * LEITURA DO TECLADO (Porta 0x60 do controlador PS/2)                       *
 * ========================================================================= */

/* Lê o scancode enviado pelo teclado (1 byte por tecla pressionada/soltada) */
static unsigned char read_scan_code(void) {
    return inb(0x60);
}

/* ========================================================================= *
 * MAPA DE SCANCODES → CARACTERES (Layout US QWERTY)                         *
 * ========================================================================= */

/* Mapa minúsculas: converte o scancode Set 1 para o caractere correspondente */
static unsigned char kbd_map_lower[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

/* Mapa maiúsculas/Shift: caracteres quando Shift está pressionado */
static unsigned char kbd_map_upper[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' '
};

/* Estado da tecla Shift (1 = pressionada, 0 = solta) */
static unsigned char shift_held = 0;

/* Scancodes das teclas Shift (esquerdo e direito) */
#define SCANCODE_LSHIFT_PRESS   0x2A
#define SCANCODE_RSHIFT_PRESS   0x36
#define SCANCODE_LSHIFT_RELEASE 0xAA
#define SCANCODE_RSHIFT_RELEASE 0xB6

/* ========================================================================= *
 * HANDLER PRINCIPAL DAS INTERRUPÇÕES                                        *
 * ========================================================================= */

void interrupt_handler(struct cpu_state cpu, unsigned int interrupt, struct stack_state stack) {
    (void)cpu;

    /* Exceções da CPU (0-31): imprime o erro e trava */
    if (interrupt < 32) {
        klog_write("\n!!! EXCECAO DA CPU: ");
        klog_write(exception_names[interrupt]);
        klog_write(" (int ");
        klog_write_hex(interrupt);
        klog_write(")\n");
        klog_write("  EIP: ");
        klog_write_hex(stack.eip);
        klog_write("  CS: ");
        klog_write_hex(stack.cs);
        klog_write("  Error code: ");
        klog_write_hex(stack.error_code);
        klog_write("\n");
        /* Trava o sistema - não há como recuperar */
        while (1) {
            __asm__("hlt");
        }
    }

    /* IRQ7 (interrupt 39): Spurious interrupt - ignora */
    if (interrupt == 39) {
        return;
    }

    if (interrupt == 33) {
        unsigned char scancode = read_scan_code();

        /* Rastreia o estado da tecla Shift (pressionada/solta) */
        if (scancode == SCANCODE_LSHIFT_PRESS || scancode == SCANCODE_RSHIFT_PRESS) {
            shift_held = 1;
            pic_acknowledge(interrupt);
            return;
        }
        if (scancode == SCANCODE_LSHIFT_RELEASE || scancode == SCANCODE_RSHIFT_RELEASE) {
            shift_held = 0;
            pic_acknowledge(interrupt);
            return;
        }

        /* Só processa teclas pressionadas (bit 7 = 0), ignora release */
        if (!(scancode & 0x80)) {
            char letra = shift_held ? kbd_map_upper[scancode] : kbd_map_lower[scancode];
            if (letra != 0) {
                /* Usa klog_write para enviar para tela + serial de uma vez */
                char buf[2] = { letra, '\0' };
                klog_write(buf);
            }
        }

        pic_acknowledge(interrupt);
    }
}