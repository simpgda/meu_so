#ifndef INCLUDE_SERIAL_H
#define INCLUDE_SERIAL_H

/* Endereço padrão de I/O da porta COM1 na arquitetura x86 */
#define SERIAL_COM1_BASE 0x3F8

/* Funções de inicialização e envio de dados para o arquivo de log */
void serial_configure_baud_rate(unsigned short com, unsigned short divisor);
void serial_configure_line(unsigned short com);
void serial_configure_buffers(unsigned short com);
void serial_configure_modem(unsigned short com);
int serial_is_transmit_fifo_empty(unsigned int com);
void serial_write(unsigned short com, char *buf, unsigned int len);
void serial_init(void);

#endif