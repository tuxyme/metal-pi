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
#include "framebuffer.h"
#include "psf.h"
#include <string.h>
#include <utils.h>
#include <mailbox.h>
#include <uart.h>

extern uint8_t _binary_libs_tuxy_250x280_rgb_start;
extern uint8_t _binary_libs_tuxy_250x280_rgb_end;

extern uint32_t _binary_libs_ter_u22n_psfu_start;
extern uint32_t _binary_libs_ter_u22n_psfu_end;

struct font_wrapper wrapper;
mb_mail mail;

void program(uint32_t r0, uint32_t r1, uint32_t atags) 
{  
   uint32_t i;

   // initialize framebuffer with default font (Terminus 16)
   framebuffer_init();
   // no cursor, no line wipe
   framebuffer_setfunc(0, 0);
   // get current font wrapper
   struct font_wrapper *wrapper = framebuffer_getfont();
   
   // write logo
   disp_rgb(&_binary_libs_tuxy_250x280_rgb_start, framebuffer_getwidth() / 2 - 125, 20, 250, 280);    
   // write display info
   console_write("\r\nTerminal16 font set:\r\n");        
   console_write_dec("display width: ", framebuffer_getwidth());
   console_write_dec("display height: ", framebuffer_getheight());
   // skip cursor below logo
   i = 300 / wrapper->charheight;
   while (i-- > 3) // got 3 lines on display
      console_write("\r\n");
      
   // Display Unicode and glyph for every Unicode in uc_map
   uint32_t chpline = (uint32_t)(framebuffer_getwidth()/(10 * wrapper->charwidth));
   uint32_t maxline = (framebuffer_getheight() - 300) / wrapper->charheight - 6;
   
   i = 0;
   while((i < wrapper->uc_size) && maxline)
   {
      framebuffer_setcolor(0xFF, 0, 0);
      uint32_t uc = wrapper->uc_map[i++] & 0xFFFF;
      console_write(htoa4(uc));
      console_write(": ");
      framebuffer_setcolor(0, 0xFF, 0);
      disp_uchar(uc);
      console_write(" ");
      
      if ((maxline == 1) && (i % chpline > chpline - 20))
      {
         console_write(" ......\r\n");
         break;
      }
      
      if (i % chpline == 0)
      {
         console_write("\r\n");
         maxline--;
      }
   }
   
   console_write("\r\n");

   // display some colors
   framebuffer_setcolor(255, 0, 0);
   console_write("*");
   framebuffer_setcolor(0, 255, 0);
   console_write("*");
   framebuffer_setcolor(0, 0, 255);
   console_write("*");
   framebuffer_setcolor(64, 64, 64);
   console_write("*");
   framebuffer_setcolor(128, 128, 128);
   console_write("*");
   framebuffer_setcolor(192, 192, 192);
   console_write("*");
   console_write("\r\n");
   
   // display board revision 
   // http://raspberryalphaomega.org.uk/2013/02/06/automatic-raspberry-pi-board-revision-detection-model-a-b1-and-b2/
   console_write_dec("Board revision: ", (hw_rev() > 3) + 1);         
   
   // Write text using Terminus 22
   loadfont(wrapper, (unsigned char*)&_binary_libs_ter_u22n_psfu_start, (unsigned char*)&_binary_libs_ter_u22n_psfu_end);
   framebuffer_setfont(wrapper);
   console_write("== And this is the Terminus 22 Font ==\r\n");
   
   /* Make a screenshot by dumping shared memory over uart
    * Wait a couple of seconds for other side to give command like this:
    *   cat /dev/ttyUSB0 > screenshot
    * At 1920x1120 24 bit color and 115200 baud the transmission takes over 7 minutes
    * Resulting file is a raw RGB image that can be imported in e.g. GIMP
      msleep(5000);
      uart_init(B115k2);
      okled(1);
      uint8_t *ptr = framebuffer_getaddr();
      uint32_t size = framebuffer_getwidth() * framebuffer_getheight() * 3;
      while (size--)
         uart_send(*ptr++);
      okled(0);
    */

}
