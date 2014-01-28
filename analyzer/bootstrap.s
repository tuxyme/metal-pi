
.section ".text.boot"

.globl _start
.globl sample
.globl waitfor

_start:
   mov sp, #0x8000
   /* initilalize bss section */
   ldr   r4, =_bss_start
   ldr   r9, =_bss_end
   mov   r5, #0
   mov   r6, #0
   mov   r7, #0
   mov   r8, #0
   b     2f
1: stmia r4!, {r5-r8}
2: cmp   r4, r9
   blo   1b
   /* enable fpu, https://github.com/dwelch67/raspberrypi */
   mrc   p15, 0, r0, c1, c0, 2
   orr   r0, r0, #0x300000
   orr   r0, r0, #0xC00000
   mcr   p15, 0, r0, c1, c0, 2
   mov   r0, #0x40000000
   fmxr  fpexc, r0
   /* start program */
   bl   program
h: bl h

   /* sample register address into buffer
    * arm timer freq set before call
    *  parameters: r0=regaddr, r1=bufstart, r2=bufsize, r3=delay
    *    r0 is address of 32 bit register
    *    r1 is word aligned
    *    r2 number of samples, >= 0
    */ 
sample:
   push  {r4-r6}
   ldr   r4, =0x2000B420   @ARM free running timer LO register
   ldr   r5, [r4]
   mov   r6, r5
   add   r6, r6, r3
L: cmp   r2, #0
   beq   2f
   ldr   r12, [r0]
   str   r12, [r1]
   add   r1, r1, #4
   sub   r2, r2, #1
1: ldr   r5, [r4]
   cmp   r5, r6
   blt   1b
   add   r6, r6, r3
   b     L
2: pop   {r4-r6}
   mov   pc, lr

   /* wait for trigger condition 
    * r0 is probe register
    * r1 is probe register trigger mask
    * r2 is probe register trigger values
    */
   
waitfor:
   ldr   r12, [r0]
   and   r12, r12, r1
   cmp   r12, r2
   bne   waitfor
   mov   r0, r12
   mov   pc, lr
   
