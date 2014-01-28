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

#include "include/mailbox.h"
#include "include/misc.h"

// GPU channel
#define MBOXREAD 0x2000B880 // bit0..3: channel ID, bit4..31: data
#define MBOXPEEK 0x2000B890 // read without state change
#define MBOXSNDR 0x2000B894
#define MBOXSTAT 0x2000B898 // bit31=0: ready for write, bit30=0: ready for read
#define MBOXCONF 0x2000B89C
#define MBOXWRTE 0x2000B8A0 // bit0..3: channel ID, bit4..31: data

void mail_send(volatile mb_mail *mail, unsigned int mailbox)
{
    mbox_write((unsigned int)(mail + 0x40000000), mailbox);
}

void mbox_write(unsigned int message, unsigned int mailbox)
{
   mailbox &= 0xF;

   do { 
      DMB(); 
   } while (GET32(MBOXSTAT) & 1<<31);   // Check that the top bit is set
    
    DMB();
    PUT32(MBOXWRTE, message | mailbox); // Combine message and mailbox channel and write to the mailbox
}

unsigned int mail_wait(volatile mb_mail *mail, unsigned int mailbox)
{
   if (mbox_read(8) == 0)
      return 0;
   
   if (mail->type != 0x80000000) 
      return 0;
   
   return 1;
}

unsigned int mbox_read(unsigned int mailbox)
{
    unsigned int status;
    mailbox &= 0xF;
    
    while(1)
    {
      do {
         DMB();
      } while (GET32(MBOXSTAT) & 1<<30);   // Check that the 30th bit is set

      DMB();
      status = GET32(MBOXREAD);

      if(mailbox == (status & 0x0F))
         return status & ~0x0F; // >>4?
    }
}

void mail_init(volatile mb_mail *mail, int tagcount)
{
   int i, j;
   
   mail->bufsize = 12 + 28 * tagcount;     // size including end tag
   mail->type = 0;         // request
   for (i = 0; i < 5; i++) // tags
   {
      mail->tag[i].tagid = 0;    // changed externally
      mail->tag[i].valsize = 16; // fixed
      mail->tag[i].actsize = 0;  // changed externally
      for (j = 0; j < 4; j++)
         mail->tag[i].val[j] = 0;
   }
}



