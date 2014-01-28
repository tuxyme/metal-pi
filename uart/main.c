
#include "uart.h"
#include <string.h>
#include <utils.h>
#include <misc.h>

int program(void)
{  
   uart_init(B115k2); 

   okled(1);

   uart_send((uint32_t)'a');
   uart_send((uint32_t)'b');
   uart_send((uint32_t)'c');
   uart_send((uint32_t)'d');
   uart_send((uint32_t)'e');
   uart_send((uint32_t)'f');
   uart_send((uint32_t)'g');
   uart_send((uint32_t)'h');
   
   while(1)
   {
      okled(1);
      uart_write("\r\n");
      uart_write("dit is een string\r\n");
      uart_write("dit is 0x1D6F: ");
      uart_write(htoa(0x1D6F));
      uart_write("\r\ndit is 639: ");
      uart_write(itoa(639));
      uart_write("\r\n");
      msleep(500);
      okled(0);
      msleep(1500);
   }
   
   return(0);
}
 
