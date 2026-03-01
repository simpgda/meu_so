#include "idt.h"
#include "io.h"

/* Função principal chamada pela CPU sempre que ocorrer uma interrupção mascarável */
void interrupt_handler(struct cpu_state cpu, struct stack_state stack, unsigned int interrupt)
{
    /* Casting para void impede que o GCC (com a flag -Werror) trave a compilação 
       alegando que essas variáveis foram recebidas mas não foram utilizadas ainda. */
    (void)cpu; 
    (void)stack;

    /* O teclado é conectado à linha IRQ 1. Como remapeamos o PIC para começar no 32,
       a interrupção do teclado é 32 + 1 = 33. */
    if (interrupt == 33) {
        /* LER a porta do teclado é OBRIGATÓRIO, mesmo se não formos imprimir a letra na tela.
           Se não lermos, o buffer do teclado enche e ele para de enviar novas interrupções. */
        unsigned char scan_code = read_scan_code();
        
        /* Ignora a variável temporariamente para evitar erro no compilador */
        (void)scan_code;

        /* Avisa o chip PIC que já terminamos de lidar com o teclado e que ele 
           pode liberar o envio de novas teclas (End of Interrupt). */
        pic_acknowledge(interrupt);
    }
}