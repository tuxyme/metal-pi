
#include <string.h>
#include <utils.h>
#include <uart.h>
#include <misc.h>

int program(void)
{
   uart_init(B115k2); 
   
   // excption als FPU niet is geinitialiseerd
   uart_write(itoa(1.0f/(float)getint()));   
   
   return(0);
}
