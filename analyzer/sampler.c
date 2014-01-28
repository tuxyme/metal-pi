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

/*
 * Free running 1 GPIO register sampling:
 * 32768 samples using GET32 calls take 13100 timer ticks = 2.5MHz
 * 25000 samples using uint ptr take 4580 timer ticks = 5.46MHz
 * sampling synchronized by ARM timer: up to 3MHz
 * sampling with readcnt4 > delaycnt4: up to 2MHz?
 * 
 * TODO: 
 *  - readcnt4 > delaycnt4 (returning samples before trigger event)
 *  - what to do with current info dump to framebuffer - needed for debug only
 *  - proper interface + buttons -> standalone logic analyzer
 */

extern unsigned int waitfor(unsigned int, unsigned int, unsigned int);
extern void sample(unsigned int, unsigned int*, unsigned int, unsigned int);

#include "sampler.h"
#include <framebuffer.h>
#include <string.h>
#include <utils.h>
#include <uart.h>
#include <misc.h>
#include <gpio.h>

#define GPFSEL0 0x20200000
#define GPFSEL1 0x20200004
#define GPFSEL2 0x20200008

#define GPSET0 0x2020001C
#define GPCLR0 0x20200028

#define GPLEV0 0x20200034
#define GPEDS0 0x20200040

#define GPREN0 0x2020004C
#define GPFEN0 0x20200058
#define GPHEN0 0x20200064
#define GPLEN0 0x20200070

#define GPPUD  0x20200094

#define SYSTCLO 0x20003004
#define SYSTCHI 0x20003008

#define ARM_TIMER_LOD 0x2000B400
#define ARM_TIMER_VAL 0x2000B404
#define ARM_TIMER_CTL 0x2000B408
#define ARM_TIMER_RLD 0x2000B418
#define ARM_TIMER_DIV 0x2000B41C
#define ARM_TIMER_CNT 0x2000B420
   
#define CM_GP0CTL (*(volatile uint32_t *)0x20101070)
#define CM_GP0DIV (*(volatile uint32_t *)0x20101074)

static uint32_t buffer[BUFSIZE];
static uint32_t bufidx; // points to byte after last written byte
static int pins[PROBECNT];


void sampler_init()
{
   int i;
   uint32_t rev = (hw_rev() > 3) + 1; // board revision 1 or 2
   
   bufidx = 0;
   
   for (i = 0; i < 4; i++)
   {
      trigger[i].values  = 0;
      trigger[i].mask    = 0;
      trigger[i].delay   = 0;
      trigger[i].level   = 0;
      trigger[i].channel = 0;
      trigger[i].serial  = 0;
      trigger[i].start   = 0;
   }
   
   /* By limiting to GPIO pins 0..31 all data is in 1 GPIO register
    * 
    *   SUMP pin | 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
    * -----------|-----------------------------------------------
    * rev 1 GPIO | 7, 8,11, 9,25,10,24,23,22,21,18,17,17,17,17,17 (12 probes)
    * rev 2 GPIO | 7, 8,11, 9,25,10,24,23,22,27,18,17,28,29,30,31 (16 probes)
    * 
    * That leaves 3 GPIO pins on P1 header (for future standalone operation):
    *         pin 4 (GPCLK0)
    *  rev 1: pin 0,1 (I2C)
    *  rev 2: pin 2,3 (I2C)
    */ 
   pins[0] = 7;
   pins[1] = 8;
   pins[2] = 11;
   pins[3] = 9;
   pins[4] = 25;
   pins[5] = 10;
   pins[6] = 24;
   pins[7] = 23;
   pins[8] = 22;
   pins[9] = rev == 1 ? 21 : 27;
   pins[10] = 18;
   pins[11] = 17;
   pins[12] = rev == 1 ? 17 : 28;
   pins[13] = rev == 1 ? 17 : 29;
   pins[14] = rev == 1 ? 17 : 30;
   pins[15] = rev == 1 ? 17 : 31;   
   
   for (i = 0; i < PROBECNT; i++)
      pin_set_func(pins[i], 0);   
   
   // activate I2C clock as test signal
   pin_set_func(rev == 1 ? 0 : 2, 4);
   pin_set_func(rev == 1 ? 1 : 3, 4);
     
   /* 200KHz testsignal on GPIO 4
    * for DIVI = 1000 (max 4095):
    *   SRC=4 (PLLB) --> Bonk
    *   SRC=5 (PLLC) --> 1.0 MHz?
    *   SRC=6 (PLLD) --> 0.5 MHz?
    *  
    * 200 KHz clock --> SRC=6 DIVI=2500 
    * 
    */
   CM_GP0CTL = CM_GP0CTL & ~0x10;       // set ENAB=0
   while (CM_GP0CTL & 0x80)             // wait for BUSY=0
      usleep(5); 
   CM_GP0DIV = (0x5a<<24) | (2500<<12); // clock div = 2500
   CM_GP0CTL = (0x5a<<24) | 0x06;       // Source=PLLD (500MHz?)
   CM_GP0CTL = (0x5a<<24) | 0x16;       // Same + ENAB=1
   while (!(CM_GP0CTL & 0x80))          // wait for BUSY=1
      usleep(5); 
   
   pin_set_func(4,4);   
   
}

uint8_t to_packet(uint32_t regv)
{
   int i;
   static uint8_t probe = 0;
   
   for (i = PROBECNT - 1; i >= 0; i--)
   {  
      probe <<= 1;
      probe |= ((regv >> pins[i])&1);
   }
   
   return probe;
}

uint32_t to_register(uint32_t probe)
{
   int i;
   uint32_t regv = 0;
   
   for (i = 0; i < PROBECNT; i++)
         regv |= ((probe >> i)&1) << pins[i];
   
   return regv;   
}

float abs(float value)
{
   return value < 0 ? -value : value;
}

void sampler_go()
{
   uint32_t samplecnt, idx, t0, t1, div, delay, packet;
   float freq;
      
   samplecnt = sampler.readcnt4 * 4;

   console_write_dec("Sampler - number of samples requested: ", samplecnt);
   samplecnt = samplecnt > BUFSIZE ? BUFSIZE : samplecnt;
   console_write_dec("Sampler - number of samples accepted: ", samplecnt);
   
   console_write_dec("Sampler - 100MHz divider requested: ", sampler.divider);
   sampler.divider = sampler.divider == 0 ? 1 : sampler.divider;
   freq = 100000000.0f / (float)sampler.divider;
   console_write_dec("Sampler - sampling freq requested (Hz): ", (uint32_t)freq);

   // div = 250MHz clock divider: 0xF9 = 1 MHz, 0x7C = 2 MHz, 0x52 = 3 MHz (max)
   // delay = number of ticks between samples

   // default 1 MHz
   div = 0xF9;
   delay = 1;
   
   if (freq > 2.5E06)
   {
      div = 0x52;
      delay = 1;
   }
   else if (freq > 1.5E06)
   {
      div = 0x7C;
      delay = 1;
   }
   else if (freq > 0.85E06)
   {
      div = 0xF9;
      delay = 1;
   }
   else
   {
      div = 0xF9;
      delay = 1;
      float delta = 3E6;
      uint32_t n = 1;
      
      // Find sample rate closest to requested freq. 
      // TODO: calculate optimal clock instead of 1/2/3 MHz steps
      
      // 1 MHz clock
      n = (int)(1E06/freq);
      if (n > 0 && (abs(1E06/n - freq) < delta))
      {
         delta = abs(1E06/n - freq);
         div = 0xF9;
         delay = n;
      }
      n++;
      if (n > 0 && (abs(1E06/n - freq) < delta))
      {
         delta = abs(1E06/n - freq);
         div = 0xF9;
         delay = n;
      }
      // 2 MHz clock
      n = (int)(2E06/freq);
      if (n > 0 && (abs(2E06/n - freq) < delta))
      {
         delta = abs(2E06/n - freq);
         div = 0x7C;
         delay = n;
      }
      n++;
      if (n > 0 && (abs(2E06/n - freq) < delta))
      {
         delta = abs(2E06/n - freq);
         div = 0x7C;
         delay = n;
      }
      // 3 MHz clock
      n = (int)(3E06/freq);
      if (n > 0 && (abs(3E06/n - freq) < delta))
      {
         delta = abs(3E06/n - freq);
         div = 0x52;
         delay = n;
      }
      n++;
      if (n > 0 && (abs(3E06/n - freq) < delta))
      {
         delta = abs(3E06/n - freq);
         div = 0x52;
         delay = n;
      }
   }

   console_write_dec("Sampler - sampling freq accepted (Hz): ", (uint32_t)(250E6/(div+1)/delay));
   console_write_dec("Sampler - ..using clock (Hz) ", (uint32_t)(250E6/(div+1)));
   console_write_dec("Sampler - ..using loop count ", delay);
      
   if (bufidx == 0)
   {      
      // Set ARM timer
      uint32_t enable = 0x200;
      PUT32(ARM_TIMER_CTL, (div << 16) | enable);

      if (trigger[0].mask)
      {
         console_write_bin("Sampler - trigger mask  : ", trigger[0].mask);
         console_write_bin("Sampler - trigger values: ", trigger[0].values);         
         enable = waitfor(GPLEV0, to_register(trigger[0].mask), to_register(trigger[0].values));
         console_write_bin("Sampler - trigger event : ", to_packet(enable));
      }

      okled(1);
      t0 = GET32(SYSTCLO);
      sample(GPLEV0, buffer, samplecnt, delay);  
      t1 = GET32(SYSTCLO);
      okled(0);
      
      t0 = (t1 - t0);
      if (t0)
      {
         freq = 1000000.0f / ((float)t0 / (float)samplecnt);
         console_write_dec("Sampler - effective sampling freq (Hz): ", (uint32_t)freq);
      }

      idx = 0; 
      t0 = 1;
      while (idx < samplecnt)
      {
         if (idx % 500 == 0)
         {
            t0 = !t0;
            okled(t0);
         }
         packet = to_packet(buffer[idx++]);
         if ((sampler.groups & 1) == 0)
            uart_send(packet & 0xFF);
         if ((sampler.groups & 2) == 0)
            uart_send((packet >> 8) & 0xFF);
         if ((sampler.groups & 4) == 0) 
            uart_send((packet >> 16) & 0xFF);
         if ((sampler.groups & 8) == 0) 
            uart_send((packet >> 24) & 0xFF);
      }
      okled(0);
      bufidx = 0;
   }
   
   console_write("Sampler - done\r\n");
}

