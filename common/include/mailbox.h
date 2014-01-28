#ifndef _MAILBOX_H_
#define _MAILBOX_H_



/* 
 * Mail template used to send data between VC and ARM
 * For available message tags and values see spec in Firmware Wiki:
 *   https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
 */

typedef struct mb_tag // 28 bytes
{
   unsigned int tagid;   // message tag
   unsigned int valsize; // initialized to 16, should stay 16. Size of value buffer in bytes
   unsigned int actsize; // total size in bytes of values used, MSB: 0=request, 1=response
   unsigned int val[4];  // 16 byte value buffer
} 
   mb_tag;

typedef struct mb_mail
{
   unsigned int bufsize; // size of complete message (including end tag) in bytes
   unsigned int type;    // 0: request, MSB=1: response, LSB=1: error
   mb_tag tag[5];        // 5 message tags, including endtag with tag.tagid = 0
}  
   mb_mail __attribute__ ((aligned(16)));

void mail_init(volatile mb_mail *mail, int tagcount);
void mail_tag(volatile mb_tag *tag);
void mail_copy(volatile mb_mail *mail, volatile mb_tag *tag);

void mail_send(volatile mb_mail *mail, unsigned int mailbox);
void mbox_write(unsigned int message, unsigned int mailbox);

unsigned int mail_wait(volatile mb_mail *mail, unsigned int mailbox);
unsigned int mbox_read(unsigned int mailbox);

#endif // _MAILBOX_H_
