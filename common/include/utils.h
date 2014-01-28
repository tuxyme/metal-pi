
#ifndef _UTILS_H
#define _UTILS_H

#include <stdint.h>

#define SYSTCLO 0x20003004
#define SYSTCHI 0x20003008

void usleep(unsigned int us);
void msleep(unsigned int ms);

void morse_chr(char c);  // sends character in morse code on OK LED 
void morse_str(char *s); // sends string in morse code on OK LED 

void okled(int on);

/*
 * @return board revision (>0) or 0 on error. 
 * Rev 1: hw_rev() <= 3. For list of revision numbers see
 *   http://raspberryalphaomega.org.uk/2013/02/06/automatic-raspberry-pi-board-revision-detection-model-a-b1-and-b2/
 */
uint32_t hw_rev();

unsigned int getint(); // some integer - not random, just to humor the compiler


#endif // _UTILS_H