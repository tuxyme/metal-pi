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

#include <stdint.h>
#include <stdarg.h>

#include "include/string.h"

void *memset(void *ptr, int set, size_t size)
{
    char *p=ptr;
    
    while(size--){
        *p++ = (int)set;
    }
    
    return p;
}

void *memcpy(void *dest, const void *src, size_t count)
{
	char *tmp = dest;
	const char *s = src;
    
	while (count--)
		*tmp++ = *s++;
	return dest;
}

char *itoa(int val)
{
	static char ascii_val[12];
	int pos=0;
   int temp;

   if (val == 0)
   {
      ascii_val[0] = '0';
      ascii_val[1] = 0;
      return ascii_val;
   }

	if(val<0)
   {
		pos++;
      val = -val;
      ascii_val[0] = '-';
   }

   // First round is to get the length
   temp = val;
	while(temp && (pos<=10)){
		temp = temp/10;
		pos++;
	}

	ascii_val[pos] = 0;

	// Second round is to do the actual conversion
	while(val && pos--){
		ascii_val[pos] = 0x30 + (val%10);
		val = val/10;
	}

	return ascii_val;
}

char *utoa(unsigned int val)
{
   static char ascii_val[12];
   int pos=0;
   unsigned int temp;

   if (val == 0)
   {
      ascii_val[0] = '0';
      ascii_val[1] = 0;
      return ascii_val;
   }

   // First round is to get the length
   temp = val;
   while(temp && (pos<=10)){
      temp = temp/10;
      pos++;
   }

   ascii_val[pos] = 0;

   // Second round is to do the actual conversion
   while(val && pos--){
      ascii_val[pos] = 0x30 + (val%10);
      val = val/10;
   }

   return ascii_val;
}

char *btoa(unsigned int val)
{
   static char ascii_val[33];
   int pos = 0;

   while (pos < 32)
   {
      ascii_val[31-pos] = 0x30 + (val & 0x01);
      val >>= 1;
      pos++;
   }

   ascii_val[32] = 0;
   
   return ascii_val;
}

char *htoa(unsigned int val)
{
   static char ascii_val[11];
   ascii_val[0] = '0';
   ascii_val[1] = 'x';
   
   int pos = 0;
   int h;

   while (pos < 8 )
   {
      h = (val & 0x0F);
      ascii_val[9-pos++] = h < 10 ? h + 0x30 : h + 0x37;
      val >>= 4;
   }

   ascii_val[10] = 0;
   
   return ascii_val;
}

char *htoa4(unsigned int val)
{
   static char ascii_val[7];
   ascii_val[0] = '0';
   ascii_val[1] = 'x';
   
   int pos = 0;
   int h;

   while (pos < 4 )
   {
      h = (val & 0x0F);
      ascii_val[5-pos++] = h < 10 ? h + 0x30 : h + 0x37;
      val >>= 4;
   }

   ascii_val[6] = 0;
   
   return ascii_val;
}

// ASCII
void lower(char *str)
{
   char *ptr = str;
   
   while (*ptr != 0)
   {
      if (*ptr >= 65 && *ptr <= 90)
         *ptr += 32;
      ptr++;
   }
}

// ASCII
void upper(char *str)
{
   char *ptr = str;
   
   while (*ptr != 0)
   {
      if (*ptr >= 97 && *ptr <= 122)
         *ptr -= 32;
      ptr++;
   }
}

