
#ifndef UART_H
#define UART_H

#include <stdint.h>

/* 
 * Requires config.txt with init_uart_clock=16000000 
 * 
 * Mini UART gaf problemen met Sigrok, waarschijnlijk overrun aan PI kant, maar de 
 * registerbits die daarop moeten wijzen worden niet geset. Gewone UART werkte beter, 
 * en geeft wel correcte foutbits (steeds overrun). Verholpen met lokale fifo.
 */

typedef enum 
{
   B115k2 = 115200, 
   B230k4 = 230400,
   B460k8 = 460800
} Baudrate;

void uart_init(Baudrate baudrate);

uint32_t uart_recv(void);
void uart_send(uint8_t c);
void uart_send32(uint32_t c);
void uart_write(const char *line);

/* UART read error callback function(byte)
 * parameter is bit 23-16 of DR register
 * (fifo overflow, etc)
 */
void uart_errcb(void (*cbfunc)(uint8_t));

#endif