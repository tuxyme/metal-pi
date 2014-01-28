/*  
 * Copyright (c) 2014 tuxyme@tuxbabe.eu
 * 
 *  Permission is hereby granted, free of charge, to any person
 *  obtaining a copy of this software and associated documentation
 *  files (the "Software"), to deal in the Software without
 *  restriction, including without limitation the rights to use,
 *  copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following
 *  conditions:
 * 
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 * 
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *  OTHER DEALINGS IN THE SOFTWARE.
 * 
 */

#include <utils.h>
#include "uart.h"
#include <misc.h>
#include <gpio.h>

#define UART_DR   0x20201000
#define UART_FR   0x20201018
#define UART_IBRD 0x20201024
#define UART_FBRD 0x20201028
#define UART_LCRH 0x2020102C
#define UART_CR   0x20201030
#define UART_ICR  0x20201044
#define GPIO_PUD  0x20200094
#define GPIO_CLK  0x20200098

#define FIFO_SIZE 1024

/*
 * This implementation uses a local fifo as the UART fifo 
 * frequently overflowed (indicated by UART_DR flags)
 */
static char fifo[FIFO_SIZE];
static uint32_t fifo_start;// points to first value in fifo
static uint32_t fifo_stop; // points to valid byte after last value in fifo

static void (*errorcb)(uint8_t);

int fifo_push(uint8_t value)
{
   if ( fifo_stop == fifo_start - 1 || 
      ( fifo_stop == FIFO_SIZE-1 && fifo_start == 0 )) // full
      return -1;
      
   fifo[fifo_stop] = value;
   fifo_stop++;
   if (fifo_stop >= FIFO_SIZE)
      fifo_stop = 0;
   
   return 0;
}

int fifo_pop()
{
   static uint8_t val;
   
   if ( fifo_stop == fifo_start ) // empty
      return -1;
   
   val = fifo[fifo_start];
   fifo_start++;
   if (fifo_start >= FIFO_SIZE)
     fifo_start = 0;
   
   return val;
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

// error callback
void uart_errcb(void (*cbfunc)(uint8_t))
{
   errorcb = cbfunc;
}

uint32_t __uart_recv(void)
{
   uint32_t reg; 
   
    while(1)
        if((GET32(UART_FR)&0x10)==0) break;
    
    reg = GET32(UART_DR);
    if ((reg & 0xF00) && errorcb)
       errorcb((reg & 0xF00)>>8);

    return(reg & 0xFF);
}

uint32_t uart_recv(void)
{
   uint32_t reg; 
   
   // empty the uart fifo in local fifo
   while ((GET32(UART_FR)&0x10)==0)
   {
      reg = GET32(UART_DR);
      if ((reg & 0xF00) && errorcb)
         errorcb((reg & 0xF00)>>8);
      fifo_push(reg & 0xFF);
   }
   
   // try to get data byte from local fifo
   reg = fifo_pop();
   
   // if local fifo was empty, wait for incoming data byte
   if (reg == -1) 
      reg = __uart_recv();
   
   return reg;
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

void uart_send(uint8_t c)
{
    while(1)
    {
       if((GET32(UART_FR)&0x20)==0) break;
    }
    PUT32(UART_DR,c);
}

//------------------------------------------------------------------------

void uart_send32(uint32_t c)
{
   uart_send(c >> 24);
   uart_send(c >> 16);
   uart_send(c >> 8);
   uart_send(c);
}

// send null terminated string
void uart_write(const char *line)
{
   int i;
   i = 0;
   
   while (line[i] != '\0')
      uart_send(line[i++]);
   
   uart_send(0);
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

void uart_init(Baudrate baudrate)
{
   errorcb = 0;
   fifo_start = 0;
   fifo_stop = 0;
   
   PUT32(UART_CR, 0);

   pin_set_func(14, 4);
   pin_set_func(15, 4);

   PUT32(GPIO_PUD, 0);
   usleep(5);
   PUT32(GPIO_CLK, (1<<14)|(1<<15));
   usleep(5);
   PUT32(GPIO_CLK, 0);

   PUT32(UART_ICR,  0x7FF);

   // UARTCLK set in config.txt: init_uart_clock=16000000
   // div = UARTCLK / ( 16 * BAUDRATE)
   //    floot(div) --> UART_IBRD
   //    frac(div) * 64 + 0.5 --> UART_FBRD
   //
   // Testing with PL2303 USB to serial on Linux:
   // 16000000 115200  8 44 tested OK
   // 16000000 230400  4 22 tested OK
   // 16000000 460800  2 11 tested OK
   
   switch (baudrate)
   {
      case B115k2:
            PUT32(UART_IBRD, 8);
            PUT32(UART_FBRD, 44);
         break;
      case B230k4:
            PUT32(UART_IBRD, 4);
            PUT32(UART_FBRD, 22);
         break;
      case B460k8:
            PUT32(UART_IBRD, 2);
            PUT32(UART_FBRD, 11);
         break;
   }
         
   PUT32(UART_LCRH, 0x70);
   PUT32(UART_CR,   0x301);
}
