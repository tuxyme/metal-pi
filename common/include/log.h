
#ifndef _LOG_H
#define _LOG_H

#include <stdint.h>

/* set output for log_... calls
 * 0=no log
 * 1: uart
 * 2: framebuffer
 * 3: morse on OK LED
 */ 
void log_channel(int channel);

void log_txt(char *val);
void log_int(int val);
void log_uint(unsigned int val);
void log_hex(uint32_t val);
void log_bin(uint32_t val);

void log_nv_txt(char *label, char *val);
void log_nv_uint(char *label, unsigned int val);
void log_nv_int(char *label, int val);
void log_nv_hex(char *label, uint32_t val);
void log_nv_bin(char *label, uint32_t val);

void log_chr(char ch);
void log_crlf();

#endif
