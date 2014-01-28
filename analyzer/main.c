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

#include <string.h>
#include <framebuffer.h>
#include "sampler.h"
#include <uart.h>
#include <misc.h>

#define SYSTCLO 0x20003004
#define BAUDRATE B115k2
//B115k2
//B460k8

void command_sendid();
void command_reset();
void command_metadata();

/*
 * Commands are 1 or 5 bytes long
 *
 * 1 byte commands:
 * 
 * 0x01: arm (pun not intended) trigger
 * 0x02: send id 1ALS
 * 0x11: Xon (resume sending data)
 * 0x13: Xoff (suspend sending data)
 * 
 * 5 byte commands:
 * 
 * 0x00 5 times:  reset
 * 0xC0 x x x x: stage 1 trigger mask
 * 0xC4 x x x x: stage 2 trigger mask
 * 0xC8 x x x x: stage 3 trigger mask
 * 0xCC x x x x: stage 4 trigger mask
 *   xxxx: channels or samples (ser or par mode)
 * 
 * 0xC1 x x x x: stage 1 trigger value
 * 0xC5 x x x x: stage 2 trigger value
 * 0xC9 x x x x: stage 3 trigger value
 * 0xCD x x x x: stage 4 trigger value
 *   xxxx: channels or samples (ser or par mode)
 *
 * 0xC2 x x x x: stage 1 trigger configuration
 * 0xC6 x x x x: stage 2 trigger configuration
 * 0xCA x x x x: stage 3 trigger configuration
 * 0xCE x x x x: stage 4 trigger configuration
 *   bit 00..15   : delay
 *   bit 16-19, 31: channels
 *   bit 22,23    : level
 *   bit 28       : start
 *   bit 29       : serial
 * 
 * 0x80: divider
 *   bit 00..23: sampling frequency = clock / value 
 * 
 * 0x81: read & delay count
 *   bit 00..15: read count : total samples returned
 *   bit 16..31: delay count: samples after the trigger
 * 
 * 0x82: flags
 *   bit 0: 1 = demux enable
 *   bit 1: 1 = filter enable
 *   bit 2-5: disabled channel groups (channel 0-7, 8-15, 16-23, 24-31)
 *   bit 6: external enable
 *   bit 7: inverted enable
 *   
 */

static uint32_t command[5];
static uint8_t cmdidx;

void ser_input(uint32_t token)
{
   int stage;
   
   /*
   console_write("char ");
   console_write(itoa(cmdidx));
   console_write(" = ");
   console_write(itoa(token));
   console_write("\r\n");
   */
   
   command[cmdidx] = token;
   
   if (cmdidx == 0) // possible single byte command
   {
      switch (command[0])
      {
         case 0x00:
               command_reset();
               cmdidx = -1;
            break;
         case 0x01: 
            sampler_go();
            break;
         case 0x02:
               command_sendid();
               cmdidx = -1;
            break;
         case 0x31:
            command_sendid();
            cmdidx = -1;
         break;
         case 0x04:
               command_metadata();
               cmdidx = -1;
            break;
         case 0x11:
               cmdidx = -1;
            break;
         case 0x13:
               cmdidx = -1;
            break;
      }      
   }
   
   if (cmdidx == 4) // possbile 5 byte command
   {
      switch (command[0] & ~0x0C)
      {
         case 0xC0: // trigger mask
               stage = (command[0] & 0x0C) >> 2;
               trigger[stage].mask = (command[1] & 0xFF) | (command[2] & 0xFF) << 8 | (command[3] & 0xFF) << 16 | (command[4] & 0xFF) << 24;
               console_write("Main - data: stage ");
               console_write(utoa(stage));
               console_write_bin(" trigger mask ", trigger[stage].mask);
               cmdidx = -1;
            break;
         case 0xC1: // trigger values
               stage = (command[0] & 0x0C) >> 2;
               trigger[stage].values = (command[1] & 0xFF) | (command[2] & 0xFF) << 8 | (command[3] & 0xFF) << 16 | (command[4] & 0xFF) << 24;
               console_write("Main - data: stage ");
               console_write(utoa(stage));
               console_write_bin(" trigger values ", trigger[stage].values);
               cmdidx = -1;
            break;
         case 0xC2: // trigger configuration
               stage = (command[0] & 0x0C) >> 2;
               trigger[stage].delay = (command[1] & 0xFF) | (command[2] & 0xFF) << 8;
               trigger[stage].level = command[3] & 0x03;
               trigger[stage].channel = (command[3] & 0xF0) >> 4 | (command[4] & 0x01) << 4;
               trigger[stage].serial = (command[4] & 0x04) >> 2;
               trigger[stage].start = (command[4] & 0x08) >> 3;
               console_write("Main - data: stage ");
               console_write(utoa(stage));
               console_write_dec(" trigger delay ", trigger[stage].delay);
               //console_write_dec("  trigger level: ", trigger[stage].level); // not supported
               //console_write_dec("  trigger channel: ", trigger[stage].channel); // not supported
               //console_write_dec("  trigger serial: ", trigger[stage].serial); // not supported
               //console_write_dec("  trigger start: ", trigger[stage].start); // not supported
               cmdidx = -1;
            break;
         case 0x80: // set divider - for hypothetical 100MHz clock
               sampler.divider = ((command[1] & 0xFF) | ((command[2] & 0xFF) << 8) | ((command[3] & 0xFF) << 16)) + 1;
               console_write_dec("Main - data: sampler clock divider ", sampler.divider);
               cmdidx = -1;
            break;
         case 0x81: // read & delay count
               sampler.readcnt4 = ((command[1] & 0xFF) | (command[2] & 0xFF) << 8) + 1;  // +1 not in spec, needed for sigrok-cli
               sampler.delaycnt4 = ((command[3] & 0xFF) | (command[4] & 0xFF) << 8) + 1; // +1 not in spec, needed for sigrok-cli
               console_write_dec("Main - data: sampler readcnt/4 ", sampler.readcnt4);
               console_write_dec("Main - data: sampler delaycnt/4 ", sampler.delaycnt4);
               cmdidx = -1;
            break;
         case 0x82: // flags
               sampler.demux = command[1] & 0x01;
               sampler.filter = (command[1] & 0x02) >> 1;
               sampler.groups = (command[1] & 0x3C) >> 2;
               sampler.external = (command[1] & 0x40) >> 6;
               sampler.inverted = (command[1] & 0x80) >> 7;
               console_write_bin("Main - data: sampler groups ", sampler.groups);
               //console_write_dec("Main - data: sampler demux flag ", sampler.demux);
               //console_write_dec("Main - data: sampler filter flag ", sampler.filter);
               //console_write_dec("Main - data: sampler external flag ", sampler.external);
               //console_write_dec("Main - data: sampler inverted flag ", sampler.inverted);
               cmdidx = -1;
            break;
      }      
   }
   
   if (++cmdidx > 4)
      cmdidx = 0;
}


void command_sendid()
{
   uart_send('1');
   uart_send('A');
   uart_send('L');
   uart_send('S');

   console_write("Main - command: sendid\r\n");
}

// http://dangerousprototypes.com/docs/The_Logic_Sniffer%27s_extended_SUMP_protocol
void command_metadata()
{
   uart_send(0x01);
   uart_write("Raspberry PI Logic Analyzer");

   uart_send(0x02);
   uart_write("0.1");
   
   uart_send(0x03);
   uart_write("0.1");
   
   // number of probes n numbered 0..n-1
   uart_send(0x20);
   uart_send32(PROBECNT);

   // sample memory
   uart_send(0x21);
   uart_send32(BUFSIZE);

   // dynamic memory(?)
   uart_send(0x22);
   uart_send32(BUFSIZE);

   // max sample rate (Hz)
   uart_send(0x23);
   uart_send32(3000000);
   
   uart_send(0x24);
   uart_send32(1);

   uart_send(0x00);

   console_write("Main - command: metadata\r\n");
}

void command_reset()
{
   console_write("Main - command: reset\r\n");
}

int program(void)
{
   unsigned int rec = 0;
   cmdidx = 0;
   
   framebuffer_init();
   console_write("Main - loading sampler\r\n");
   sampler_init();
   console_write("Main - loading uart\r\n");
   uart_init(BAUDRATE);
   console_write("Main - wait for input\r\n");
   
   while(1)
   {
      rec = uart_recv();
      ser_input(rec);
   }
   return(0);
}
