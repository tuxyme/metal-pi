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

#include "include/gpio.h"
#include "include/misc.h"

#define GPFSEL0 0x20200000
#define GPFSEL1 0x20200004
#define GPFSEL2 0x20200008

#define GPSET0 0x2020001C
#define GPCLR0 0x20200028

// value: 0 or 1
void pin_set(int pin, uint8_t value)
{
   value &= 1;
   pin = pin > 53 ? 53 : pin;
   
   int addr = GPSET0;
   
   if (value == 0)
   {
      addr = GPCLR0;
      value = 1;
   }
   
   if (pin > 31)
   {
      addr += 4;
      pin -= 32;
   }
   
   PUT32(addr, value << pin);
}

/* alt is pin "field" value:  
 * 0=read, 1=write, 2, 3, 4, 5, 6, 7 = alt5, alt4, alt0, alt1, alt2, alt3
 * (see BCM2835 ARM Peripherals page 102)
 */
void pin_set_func(int pin, uint8_t alt)
{   
   if (pin < 0 || pin > 53)
      return;
   
   int reg = pin / 10;
   int field = pin % 10;
   alt &= 0x07;
   
   uint32_t regaddr = GPFSEL0 + reg * 4;
   // set field to 000 for pin as input 
   uint32_t tmp = GET32(regaddr);
   tmp &= ~(0x07 << (field * 3));
   tmp |= alt << (field * 3);
   PUT32(regaddr, tmp);
} 
