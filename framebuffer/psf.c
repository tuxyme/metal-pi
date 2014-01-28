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

#include "psf.h"
#include <string.h>

#define PSF2_SEPARATOR  0xFF
#define PSF2_STARTSEQ   0xFE

/* 
 * This file parses PSF-2 font data including Unicode mapping
 * 
 * Default font included in source:
 * 
 * Terminus16 with 879 glyphs sized 16x8
 * Source : http://terminus-font.sourceforge.net
 * License: OSF License
 * 
 * To include other fonts: 
 * - search for PSF, PSFU, BDF, font, console, framebuffer
 * - check license (especially if you want to redistribute)
 * - download
 * - if distributed in bdf format convert using bdf2psf:
 *     > perl bdftopsf.pl +h -1 -o ter-u16n.psf -- ter-u16n.bdf
 *     bdftopsf.pl is included in this tarball
 *     license: GPLv2 by Dimitar Toshkov Zhekov
 * - to link PSF data into binary see Makefile
 * - to see how to use your font for framebuffer see psf.h, main.c
 * 
 * Useful links, tips, assertions:
 * - Linux console fonts showcase: 
 *     http://alexandre.deverteuil.net/consolefonts/consolefonts.html
 * - Linux console fonts are GPL unless stated otherwise 
 *     http://www.kbd-project.org
 *     or /usr/share/consolefonts
 * - Fonts covered by GPL can be usable outside of GPL:
 *     http://www.gnu.org/licenses/gpl-faq.html#FontException
 * - Interesting: Gohu font released by Hugo Chargois under WTFPL
 *     http://font.gohu.org
 * 
 * =================================================================
 * 
 * Note: doc below is taken from Linux kbd source
 * kbd home: ftp://ftp.kernel.org/pub/linux/utils/kbd/
 * 
 * This doc states that <uc> is a two-byte Unicode symbol.
 * Guess that's UCS-2 legacy, its all UTF-8 now.
 * Tested up to 3-byte UTF-8
 * 
 * ========================== QUOTE ================================
 * 
 * Format of the Unicode information
 *
 * For each font position <uc>*<seq>*<term>
 * where <uc> is a 2-byte little endian Unicode value,
 * <seq> = <ss><uc><uc>*, <ss> = psf1 ? 0xFFFE : 0xFE,
 * <term> = psf1 ? 0xFFFF : 0xFF.
 * and * denotes zero or more occurrences of the preceding item.
 *
 * Semantics:
 * The leading <uc>* part gives Unicode symbols that are all
 * represented by this font position. The following sequences
 * are sequences of Unicode symbols - probably a symbol
 * together with combining accents - also represented by
 * this font position.
 *
 * Example:
 * At the font position for a capital A-ring glyph, we
 * may have:  * 00C5,212B,FFFE,0041,030A,FFFF
 * Some font positions may be described by sequences only,
 * namely when there is no precomposed Unicode value for the glyph.
 * 
 * ========================= END QUOTE =============================
 */

struct psf2_header 
{
   unsigned char magic[4];  // 72h, B5h, 4Ah, 86h
   unsigned int version;
   unsigned int headersize;
   unsigned int flags;
   unsigned int length;   
   unsigned int charsize; 
   unsigned int height;
   unsigned int width; 
} __attribute__((packed));

struct psf2_header *psf_header;


/*
 * Decode max 3 byte UTF-8 to 16 bit Unicode value
 * Return 16 bit Unicode value or -1 for invalid data
 * Invalid data = non-UTF-8 or >3byte UTF-8
 */
int getUTF8(unsigned char **uptr, unsigned char *font_end)
{
   unsigned char *ptr = *uptr;
   int utf8 = -1;
   
   if ((*ptr & 0x80) == 0) // 1 byte UTF-8
   {
      if (ptr >= font_end)
         return -1;

      utf8 = (int)(*ptr);
      (*uptr)++;
      return utf8;
   }

   if ((*ptr & 0xE0) == 0xC0) // 2 byte UTF-8
   {
      if (ptr + 1 >= font_end)
         return -1;
      
      utf8 = (*ptr++ & ~0xE0)<<6;
      utf8 |= *ptr++ & ~0xC0;
      *uptr += 2;
      return utf8;
   }

   if ((*ptr & 0xF0) == 0xE0) // 3 byte UTF-8
   {
      if (ptr + 2 >= font_end)
         return -1;
      
      utf8 = (*ptr++ & ~0xF0)<<12;
      utf8 |= (*ptr++ & ~0xC0)<<6;
      utf8 |= *ptr++ & ~0xC0;
      *uptr += 3;
      return utf8;
   }
   
   return utf8;
}

void map_glyph(struct font_wrapper *wrapper, uint16_t unicode, uint16_t glyph_index)
{
   if (wrapper->uc_size < MAPSIZE)
      wrapper->uc_map[wrapper->uc_size++] = unicode | (glyph_index << 16);
   else
      wrapper->status |= 8;
}

int get_glyph(struct font_wrapper *wrapper, uint16_t unicode)
{
  int s1 = 0, s = 0;
  int s2 = wrapper->uc_size - 1;

  while (s2 >= s1) 
  {
      s = (s1+s2)/2;
      if (unicode < (wrapper->uc_map[s]&0xFFFF)) 
        s2 = s-1;
      else if (unicode > (wrapper->uc_map[s]&0xFFFF)) 
        s1 = s+1;
      else 
        return (wrapper->uc_map[s] & 0xFFFF0000) >> 16;
  }

  return -1;
}

/* Unicode values often in about right order, using bubblesort */
void sort(struct font_wrapper *wrapper) 
{
    uint32_t i, j, s, cnt;
    for (j = 0; j < wrapper->uc_size; j++) 
    {
       cnt = 0;
       for (i = 1; i < wrapper->uc_size - j; i++) 
       {
          if ((wrapper->uc_map[i-1]&0xFFFF) > (wrapper->uc_map[i]&0xFFFF)) 
          {
             cnt++;
             s = wrapper->uc_map[i];
             wrapper->uc_map[i] = wrapper->uc_map[i-1];
             wrapper->uc_map[i-1] = s;
          }
       }
       if (cnt == 0)
          break;
    }
}

void loadfont(struct font_wrapper *wrapper, unsigned char *font_start, unsigned char *font_end) 
{  
    wrapper->status = 0;
    
    // map psf header
    psf_header = (struct psf2_header*)font_start;      

    if ( psf_header->magic[0] != 0x72 || 
         psf_header->magic[1] != 0xB5 || 
         psf_header->magic[2] != 0x4A || 
         psf_header->magic[3] != 0x86 )
       wrapper->status |= 1;

    if (psf_header->version != 0)
       wrapper->status |= 2;
        
    // byte aligned bit count
    int bits = ((psf_header->width + 7) / 8) * 8;
    
    // initialize font wrapper
    wrapper->charwidth    = psf_header->width;
    wrapper->alignedwidth = bits;
    wrapper->charheight   = psf_header->height;
    wrapper->charcount    = psf_header->length;
    wrapper->charsize     = psf_header->charsize;
    wrapper->data         = font_start + psf_header->headersize;
    
    wrapper->uc_size = 0;

    if (wrapper->status != 0)
       return;
    
    // get unicode data    
    if (psf_header->flags & 1) // unicode data present
    {      
       int fontpos = 0;
       unsigned char *uptr;
       int utf8;
       
       uint32_t fl = psf_header->length * psf_header->charsize + psf_header->headersize;
       uptr = (unsigned char*)((uint32_t)psf_header + fl);
      
       /*
        * Supported:
        * - Unicode base plane 0x0000 - 0xFFFF
        * - encoding in 1-3 UTF-8 bytes
        * - max MAPSIZE Unicode to glyph mappings
        */
       while (uptr < font_end)
       {          
          if (*uptr == PSF2_SEPARATOR) // utf8 bytes never reach FF
          {
             fontpos++;
             uptr++;
             continue;
          } 
         
          /* ignore alternatives */
          if (*uptr == PSF2_STARTSEQ) // utf8 bytes never reach FE
          {
             while ((*uptr != PSF2_SEPARATOR) && (uptr < font_end))
               uptr++;
             continue; // defer to PSF2_SEPARATOR handling
          } 

          utf8 = getUTF8(&uptr, font_end);
               
          if (utf8 < 0)
          {
             wrapper->status |= 4;
             uptr++;
             continue;
          }

          map_glyph(wrapper, utf8, fontpos);         
       }
    }
    else // no unicode data, assume straight mapping voor ASCII chars
    {
       wrapper->status |= 16;
       wrapper->uc_size = 0;
       int i;
       for (i = 0; i <= 0xFF; i++)
          map_glyph(wrapper, i, i);
    }
    
    sort(wrapper);
}
