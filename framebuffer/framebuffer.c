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

#include "framebuffer.h"
#include "psf.h"

#include <mailbox.h>
#include <string.h>
#include <misc.h>

/*
 * Framebuffer parameters. 
 * 24bit depth
 * pitch is fb_width * depth / 8
 */
static uint32_t fb_addr, fb_size, fb_width, fb_height, fb_pitch; 

// clear line on/of, current color
uint8_t do_clr, do_cur, red, green, blue;

struct font_wrapper localwrapper;
struct font_wrapper *fontwrapper;

typedef struct cursor_t
{
   uint32_t curx;
   uint32_t cury;
   uint32_t cur_set; // cursor is written
   uint32_t clry;    // last cleared line
} cursor_t;

static cursor_t cursor = {0, 0, 0, 0};

volatile mb_mail mail;

int framebuffer_init()
{
   do_clr = 1;
   do_cur = 1;
   
   red = green = blue = 0xFF;

   fontwrapper = &localwrapper;
   loadfont( fontwrapper, 
             (unsigned char*)&_binary_libs_ter_u16n_psfu_start, 
             (unsigned char*)&_binary_libs_ter_u16n_psfu_end );   

   // get framebuffer size
   mail_init(&mail, 1);
   mail.tag[0].tagid = 0x40003; // get physical width and height of screen
   mail.tag[0].actsize = 0;     // request (MSB=0), and not sending any values
   mail.tag[0].val[0] = 0;      // width
   mail.tag[0].val[1] = 0;      // height
   mail_send(&mail, 8);
   if (!mail_wait(&mail, 8)) return -1;
   
   fb_width = mail.tag[0].val[0];
   fb_height = mail.tag[0].val[1];

   // set/set framebuffer parameters in one go as required
   mail_init(&mail, 4);
   mail.tag[0].tagid = 0x48004; // set virtual size
   mail.tag[0].actsize = 8;   
   mail.tag[0].val[0] = fb_width;    
   mail.tag[0].val[1] = fb_height;    
   mail.tag[1].tagid = 0x48005; // set depth
   mail.tag[1].actsize = 4;   
   mail.tag[1].val[0] = 24;
   mail.tag[2].tagid = 0x40001; // get framebuffer pointer
   mail.tag[2].actsize = 4;   
   mail.tag[2].val[0] = 16;    
   mail.tag[3].tagid = 0x40008; // get fb_pitch
   mail.tag[3].actsize = 0;   
   mail_send(&mail, 8);
   if (!mail_wait(&mail, 8)) return -1;

   fb_addr  = mail.tag[2].val[0];
   fb_size  = mail.tag[2].val[1];   
   fb_pitch = mail.tag[3].val[0];
  
   return 1;
}

// copy values to allow reuse of wrapper
void framebuffer_setfont(struct font_wrapper *wrapper)
{
   int i;
   
   fontwrapper->charheight = wrapper->charheight;
   fontwrapper->charwidth = wrapper->charwidth;
   fontwrapper->alignedwidth = wrapper->alignedwidth;
   fontwrapper->charsize = wrapper->charsize;
   fontwrapper->charcount = wrapper->charcount;   
   fontwrapper->uc_size = wrapper->uc_size;
   
   for (i=0; i<MAPSIZE; i++)
      fontwrapper->uc_map[i] = wrapper->uc_map[i];

   fontwrapper->data = wrapper->data;
}

void framebuffer_setcolor(uint8_t n_red, uint8_t n_green, uint8_t n_blue)
{
   red   = n_red;
   green = n_green;
   blue  = n_blue;
}

void framebuffer_setfunc(int cur, int clr)
{
   do_cur = cur;
   do_clr = clr;
}

struct font_wrapper *framebuffer_getfont(void)
{
   return fontwrapper;
}

int framebuffer_blank(uint8_t on)
{
   mail_init(&mail, 1);
   mail.tag[0].tagid = 0x40002;
   mail.tag[0].actsize = 4;
   mail.tag[0].val[0] = on & 1;
   mail_send(&mail, 8);
   if (!mail_wait(&mail, 8)) return -1;
   
   return 0;
}   

unsigned int framebuffer_getwidth()
{
   return fb_width;
}

unsigned int framebuffer_getheight()
{
   return fb_height;
}

uint32_t *framebuffer_getaddr()
{
   return (uint32_t*)fb_addr;
}

void put_pixel(uint32_t x, uint32_t y, uint8_t red, uint8_t green, uint8_t blue)
{
   uint8_t *ptr = (uint8_t*)(fb_addr + y * fb_pitch + x * 3);
   *ptr++ = red;
   *ptr++ = green;
   *ptr++ = blue;
}

void wipe(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
   uint8_t *ptr=0;
   
   while (h--)
   {
      ptr = (uint8_t*)(fb_addr + (y + h) * fb_pitch + x * 3);
      int i = w * 3;
      while (i--)
         *ptr++ = 0;
   }
}

void console_clrln()
{
   wipe(0, cursor.cury, fb_width-1, fontwrapper->charheight);
}

/*
 * clear cursor before wrap
 */
void check_wrap()
{
   if (cursor.curx + fontwrapper->charwidth >= fb_width)
   {
      // wrap to new line
      cursor.curx = 0;
      cursor.cury += fontwrapper->charheight;
      if (cursor.cury >= fb_height - fontwrapper->charheight)
         cursor.cury = 0;
      
      // clean new line
      if (do_clr && cursor.cury != cursor.clry)
      {
         wipe(0, cursor.cury, fb_width-1, fontwrapper->charheight);
         cursor.clry = cursor.cury;
      }
   }
}

void disp_cursor(uint8_t show)
{
   uint32_t x;
   
   if (!do_cur)
      return;
   
   if (show && (cursor.curx <= fb_width - fontwrapper->charwidth))
      for (x = 1; x < fontwrapper->charwidth - 1; x++)
         put_pixel(cursor.curx + x, cursor.cury + fontwrapper->charheight - 1, red, green, blue);
   
   if (!show && (cursor.curx <= fb_width - fontwrapper->charwidth))
      for (x = 1; x < fontwrapper->charwidth - 1; x++)
         put_pixel(cursor.curx + x, cursor.cury + fontwrapper->charheight - 1, 0, 0, 0);
      
   cursor.cur_set = show;
}

void disp_invalchar()
{
   uint32_t x,y;

   if (cursor.cur_set && do_cur)
      disp_cursor(0);

   check_wrap();
   
   for (y = 0; y < fontwrapper->charheight; y++)
      for (x = 0; x < fontwrapper->charwidth; x++)
         if ( x == 1 || x == fontwrapper->charwidth -2 || y == 1 || y == fontwrapper->charheight - 2 )
            put_pixel(x + cursor.curx, y + cursor.cury, red, green, blue);
         else
            put_pixel(x + cursor.curx, y + cursor.cury, 0, 0, 0);            

   cursor.curx += fontwrapper->charwidth;
}

void console_write(const char *msg)
{
   if (cursor.cur_set && do_cur)
      disp_cursor(0);
   
   for(;*msg;msg++)
   {
      if (*msg == '\r')
         cursor.curx = 0;
      else if (*msg == '\n')
      {
         cursor.cury+=fontwrapper->charheight;
         if (cursor.cury > fb_height - fontwrapper->charheight)
            cursor.cury = 0;
         if (do_clr && cursor.cury != cursor.clry)
         {
            wipe(cursor.curx, cursor.cury, fb_width, fontwrapper->charheight);
            cursor.clry = cursor.cury;
         }
      }
      else
         disp_char(*msg);
   }
   
   if (do_cur)
      disp_cursor(1);
}

void console_write_bin(const char *msg, uint32_t value)
{
   console_write(msg);
   console_write(btoa(value));
   console_write("\r\n");
}

void console_write_hex(const char *msg, uint32_t value)
{
   console_write(msg);
   console_write(htoa(value));
   console_write("\r\n");
}

void console_write_dec(const char *msg, int value)
{
   console_write(msg);
   console_write(itoa(value));
   console_write("\r\n");
}

/*
 * display ASCII char
 */
void disp_char(uint8_t cval)
{
   if (cval > fontwrapper->charcount) 
   {
      disp_invalchar();
      return;
   }
   disp_uchar((uint16_t)cval);
}

/*
 * display Unicode character 0x0000 - 0xFFFF
 */
void disp_uchar(uint16_t cval)
{
   int val = get_glyph(fontwrapper, cval); 
   
   if (val < 0) 
   {
      disp_invalchar();
      return;
   }
   disp_glyph(val);
}

/*
 * display character at index idx in font data 
 */
void disp_glyph(uint16_t idx)
{
   uint8_t *ptr = fontwrapper->data;
   uint32_t font_offset = idx * fontwrapper->charsize;
   uint32_t x,y;
   ptr += font_offset;
   uint8_t bit = 1 << 7;

   if (cursor.cur_set && do_cur)
      disp_cursor(0);

   check_wrap();
   
   if (idx >= fontwrapper->charcount) 
   {
      disp_invalchar();
      return;
   }
   
   for(y = 0; y < fontwrapper->charheight; y++)
      for(x = 0; x < fontwrapper->alignedwidth; x++) // byte aligned
      {
         if (x < fontwrapper->charwidth) // do not draw padding bits
         {
            if ((*ptr)&bit)
               put_pixel(x + cursor.curx, y + cursor.cury, red, green, blue);
            else
               put_pixel(x + cursor.curx, y + cursor.cury, 0, 0, 0);
         }
            
         bit >>= 1;
         if (!bit)
         {
            bit = 1 << 7;
            ptr++;
         }
      }
   
   cursor.curx += fontwrapper->charwidth;
}

void disp_rgb(uint8_t *ptr, uint32_t xpos, uint32_t ypos, uint32_t width, uint32_t height)
{
   uint32_t x, y, r, g, b;
   
   if (ptr == 0)
      return;
   
   for(y=0; y<height; y++)
      for(x=0; x<width; x++)
      {
         r = *ptr++; g = *ptr++; b = *ptr++; 
         put_pixel(xpos+x, ypos+y, r, g, b);
      }
}
