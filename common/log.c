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
 
#include "./include/log.h"
#include "include/string.h"
#include <uart.h>

static int channel;

void log_channel(int outch)
{
   channel = outch;
   
   if (channel == 1)
      uart_init(B115k2); 
}

void log_txt(char *val)
{
   if (channel == 1)
      uart_write(val);
}

void log_int(int val)
{
   if (channel == 1)
      uart_write(itoa(val));
}

void log_uint(unsigned int val)
{
   if (channel == 1)
      uart_write(utoa(val));
}

void log_hex(uint32_t val)
{
   if (channel == 1)
      uart_write(htoa(val));
}

void log_bin(uint32_t val)
{
   if (channel == 1)
      uart_write(btoa(val));
}

void log_chr(char ch)
{
   if (channel == 1)
      uart_send(ch);
}

void log_crlf()
{
   if (channel == 1)
      uart_write("\r\n");
}

void log_nv_txt(char *label, char *val)
{
   log_txt(label);
   log_txt(val);
   log_crlf();
}

void log_nv_int(char *label, int val)
{
   log_txt(label);
   log_int(val);
   log_crlf();
}

void log_nv_uint(char *label, unsigned int val)
{
   log_txt(label);
   log_uint(val);
   log_crlf();
}

void log_nv_hex(char *label, uint32_t val)
{
   log_txt(label);
   log_hex(val);
   log_crlf();
}

void log_nv_bin(char *label, uint32_t val)
{
   log_txt(label);
   log_bin(val);
   log_crlf();
}


