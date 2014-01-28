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
 
#include "./include/utils.h"
#include "./include/gpio.h"
#include "./include/mailbox.h"


static int utils_initstate = 0; // 1 = initialized

/* xxxyyyyy 
 *   x = length of token (1..5)
 *   y = signals (0:short 1:long)
 * 
 * Een display is voor watjes
 */
static uint8_t morsecode[37] = {
   0b01000001, //a
   0b10001000, //b
   0b10001010, //c
   0b01100100, //d
   0b00100000, //e
   0b10000010, //f
   0b01100110, //g
   0b10000000, //h
   0b01000000, //i
   0b10000111, //j
   0b01100101, //k
   0b10000100, //l
   0b01000011, //m
   0b01000010, //n
   0b01100111, //o
   0b10000110, //p
   0b10001101, //q
   0b01100010, //r
   0b01100000, //s
   0b00100001, //t
   0b01100001, //u
   0b01100001, //v
   0b01100011, //w
   0b10001001, //x
   0b10001011, //y
   0b10001100, //z
   0b10111111, //0
   0b10101111, //1
   0b10100111, //2
   0b10100011, //3
   0b10100001, //4
   0b10100000, //5
   0b10110000, //6
   0b10111000, //7
   0b10111100, //8
   0b10111110, //9
   0b11000000 //space cnt = 6 special token
}; // 73


volatile unsigned int *stimer_lo = (unsigned int*)(SYSTCLO);
volatile mb_mail mail;

void usleep(unsigned int us)
{
   unsigned int start = *stimer_lo;
   unsigned int stop = start + us;
   unsigned int now;
   
   while (us > 0)
   {
      now = *stimer_lo;
      if ( (stop > start && (now > stop || now < start)) ||
           (stop < start && (now > stop && now < start)) )
         us = 0;
   }
}

void msleep(unsigned int ms)
{
   usleep(ms * 1000); // overflow be damned
}

unsigned int getint()
{
   return *stimer_lo;
}

static void utils_init()
{
   // OK LED as output
   pin_set_func(16, 1);
   utils_initstate = 1;
}

/* 
 * @param type: 0=dot 1=dash 
 * Note: OK LED sits between GPIO16 and 3.3V so inverted
 */
static void morse_token(int type)
{
   if (type == 0)
   {
      pin_set(16, 0);
      msleep(100);
      pin_set(16, 1);
   }
      else
   {
      pin_set(16, 0);
      msleep(500);
      pin_set(16, 1);
   }
}

void morse_chr(char a)
{
   if (!utils_initstate)
      utils_init();
   
   a += (a >= 48 && a <= 57) ? 43 : 0; // map 0..9 behind Z
   a -= 65; // map char A to int 0
   a = a > 36 ? 36 : a; // limit a within morsecode array
      
   uint8_t code = morsecode[(uint8_t)a];
   
   int cnt = code >> 5;
   // cnt == 6: space
   if (cnt < 6)
   while (--cnt >= 0)
   {
      morse_token((code >> cnt) & 1);         
      msleep(200);
   }
   
   msleep(500);
}

void morse_str(char *s)
{
   while (*s != 0)
      morse_chr(*s++);
}

// @param on: 0=off 1=on
void okled(int on)
{
   if (!utils_initstate)
      utils_init();
   
   if (!on)
      pin_set(16, 1);
   else
      pin_set(16, 0);
}

uint32_t hw_rev()
{
   mail_init(&mail, 1);
   mail.tag[0].tagid = 0x10002; // board revision
   mail_send(&mail, 8);
   if (!mail_wait(&mail, 8))
      return 0;
   
   return mail.tag[0].val[0];
}