
/*
   Vector placement from blinker05 https://github.com/dwelch67/raspberrypi
*/

.section ".text.boot"

.globl _start
_start:
   ldr pc,hand_reset
   ldr pc,hand_undef
   ldr pc,hand_swi
   ldr pc,hand_prefetch
   ldr pc,hand_data
   ldr pc,hand_unused
   ldr pc,hand_irq
   ldr pc,hand_fiq
   
   hand_reset:    .word reset
   hand_undef:    .word intr_undef
   hand_swi:      .word halt
   hand_prefetch: .word halt
   hand_data:     .word halt
   hand_unused:   .word halt
   hand_irq:      .word halt
   hand_fiq:      .word halt
   
reset:
   /* copy vector data to 0x0000 */
   mov r0,#0x8000
   mov r1,#0x0000
   ldmia r0!,{r2,r3,r4,r5,r6,r7,r8,r9}
   stmia r1!,{r2,r3,r4,r5,r6,r7,r8,r9}
   ldmia r0!,{r2,r3,r4,r5,r6,r7,r8,r9}
   stmia r1!,{r2,r3,r4,r5,r6,r7,r8,r9}
   mov sp, #0x8000
   /* initilalize the bss section */
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
   /* enable fpu, commented out to cause undefined instruction exception 
   mrc p15, 0, r0, c1, c0, 2
   orr r0,r0,#0x300000
   orr r0,r0,#0xC00000
   mcr p15, 0, r0, c1, c0, 2
   mov r0,#0x40000000
   fmxr fpexc,r0
   */
   /* start program */
   bl   program
   bl   itr_hang

halt:
   b     halt

/* use SRS ? */
intr_undef: 
   mrs   r0, cpsr
   mov   r1, r14
   bl    itr_undef

