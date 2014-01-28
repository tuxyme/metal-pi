
#ifndef SAMPLER_H
#define SAMPLER_H

#include <stdint.h>
#include <stdarg.h>

#define BUFSIZE 1000000
#define PROBECNT 16

struct trigger_conf_t
{
   int delay;   // wait delay samples on match
   int level;   // ?
   int channel; // channel to use in serial mode
   int serial;  // 1: serial mode on channel
   int start;   // 1: start capture on match
   uint32_t values; // trigger values
   uint32_t mask;  // trigger mask
};

struct sampler_t
{
   int divider;  // sampling freq = clock / (divider + 1)
   int readcnt4; // total number of samples to send (divided by 4)
   int delaycnt4;// number of samples after the trigger to send (divided by 4)
   int demux;    // ?
   int filter;   // ?
   int groups;   // channels to include in transmission. Bit 0,1,2,3 each represent 8 channels
   int external; // external clock - not used
   int inverted; // external clock inverter - not used
};

struct trigger_conf_t trigger[4];
struct sampler_t sampler;

void sampler_init();
void sampler_go();


#endif