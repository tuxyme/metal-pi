
#ifndef _PSF_H
#define _PSF_H

#include <stdint.h>

#define MAPSIZE 1024 // number of 16 bit Unicode values that can be mapped to a glyph index

/* 
 * Font   : Terminus 16, 879 glyphs, 16x8 pixels
 * Source : http://terminus-font.sourceforge.net
 * License: OSF License
 */
extern uint32_t _binary_libs_ter_u16n_psfu_start;
extern uint32_t _binary_libs_ter_u16n_psfu_end;

/*
 * Value in status after loadfont call:
 *   0: all OK
 *   1: no magic (data starts with 72 B5 4A 86) (unusable)
 *   2: PSF2 version not 0 as expected (unusable)
 *   4: invalid UTF-8 data seen (can be non critical)
 *   8: uc_map MAXSIZE overflow (can be non critical)
 *  16: no unicode data (non critical)
 * 
 * In case of PSF data without Unicode (bit 4 set), uc_map will
 * contain a 1-to-1 map for ascii values: 
 *   uc_map[x] = x for x {0..255}
 *   uc_size = 256
 */
typedef struct font_wrapper
{
   uint32_t charheight;      // pixel height of glyph
   uint32_t charwidth;       // pixel width of glyph
   uint32_t alignedwidth;    // bit with of glyph (psf2 is byte-aligned)
   uint32_t charsize;        // glyph size in bytes (alignedwidth * charheight)
   uint32_t charcount;       // number of glyphs
   uint32_t uc_size;         // number of 16 bit Unicode (UCS-2) mappings in uc_map array
   uint32_t uc_map[MAPSIZE]; // sorted map UCS-2 Unicode (X) to Glyph index (Y) as 0xYYYYXXXX
   unsigned char *data;      // pointer to first glyph (of charsize bytes)
   uint32_t status;          // status of this wrapper after loadfont
} font_wrapper;

/*
 * Return font data in wrapper based on PSF data in memory
 *   wrapper   : initialized font_wrapper
 *   font_start: first byte of PSF data
 *   font_end  : first byte after PSF data
 */
void loadfont(font_wrapper *wrapper, unsigned char *font_start, unsigned char *font_end);

/*
 *  return  glyph index for 16 bit Unicode character
 *  return -1 if no mapping exist
 */
int get_glyph(font_wrapper *wrapper, uint16_t unicode);

/*
 * Decode max 3 byte UTF-8 to 16 bit Unicode value
 * Return 16 bit Unicode value or -1 for invalid data
 * Invalid data = non-UTF-8 or >3byte UTF-8
 * 
 * uptr    : pointer to UTF-8 data, passed by reference
 * font_end: first byte after font data
 * 
 * Pointer uptr is advanced to the byte after the decoded UTF-8 data
 * In case of invalid data, the pointer remains unchanged
 */
int getUTF8(unsigned char **uptr, unsigned char *font_end);

#endif