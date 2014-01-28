
/*
 * GET/PUT from https://github.com/dwelch67/raspberrypi
 */
 
.globl GETPC
GETPC:
   mov   r0, lr
   mov   pc, lr

.globl PUT32
PUT32:
   str   r1,[r0]
   bx    lr

.globl GET32
GET32:
   ldr   r0,[r0]
   bx    lr

/*
 * Data memory barrier (make sure all mem writes are observed)  writes r12 
 * see doc https://github.com/raspberrypi/firmware/wiki/Accessing-mailboxes 
 */
.globl DMB
DMB:
   mov   r12, #0
   mcr   p15, 0, r12, c7, c10, 5
   mov   pc, lr
   
/* 
 * Data synchronisation barrier (make sure all mem writes are completed) writes r12 
 */
.globl DSB
DSB:
   mov   r12, #0
   mcr   p15, 0, r12, c7, c10, 4
   mov   pc, lr
