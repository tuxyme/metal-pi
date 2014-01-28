
#ifndef _FRAMEBUFFER_H_
#define _FRAMEBUFFER_H_

#include <stdint.h>
#include "psf.h"

int framebuffer_init();

/* Set custom font loaded in wrapper. 
 * Wrapper is copied and can be reused after call
 * Default font is Terminus16
 */
void framebuffer_setfont(struct font_wrapper *wrapper);
/* set current drawing color (default white) */
void framebuffer_setcolor(uint8_t n_red, uint8_t n_green, uint8_t n_blue);
/* set cursor and clear line functions (default on) */
void framebuffer_setfunc(int cur, int clr);
/* clear screen */
int framebuffer_blank(uint8_t on);

struct font_wrapper *framebuffer_getfont(void);
unsigned int framebuffer_getwidth();
unsigned int framebuffer_getheight();
uint32_t *framebuffer_getaddr();

void console_write(const char *msg);
void console_clrln(); // clear current line
void disp_char(uint8_t cord);    // draw ASCII character
void disp_uchar(uint16_t cord);  // draw Unicode character
void disp_glyph(uint16_t cord);  // draw character on zero based index in font data
void disp_rgb(uint8_t *ptr, uint32_t xpos, uint32_t ypos, uint32_t width, uint32_t height);

void console_write_bin(const char *msg, uint32_t value);
void console_write_hex(const char *msg, uint32_t value);
void console_write_dec(const char *msg, int value);

void wipe(uint32_t x, uint32_t y, uint32_t w, uint32_t h);

#endif // _FRAMEBUFFER_H_

