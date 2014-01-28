
#ifndef _GPIO_H
#define _GPIO_H

#include <stdint.h>

// value: 0 or 1
void pin_set(int pin, uint8_t value);
// alt is pin "field" value: 0=read, 1=write, 2=pwm (see bcm doc)
void pin_set_func(int pin, uint8_t alt);
 
#endif
