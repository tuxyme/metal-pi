
#include <utils.h>
#include <string.h>

void itr_undef(unsigned int cpsr, unsigned int lr)
{ 
   while (1)
   {      
      char *ptr = " UND ";
      while (*ptr != '\0')
         morse_chr(*ptr++);

      msleep(1000);
      
      ptr = htoa(lr);
      while (*ptr != '\0')
         morse_chr(*ptr++);

      msleep(1000);
   }
}

// Send morse K forever
void itr_hang()
{
      while (1)
         morse_str("K  ");
}

