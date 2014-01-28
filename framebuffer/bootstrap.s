
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
   hand_swi:      .word intr_hang
   hand_prefetch: .word intr_hang
   hand_data:     .word intr_hang
   hand_unused:   .word intr_hang
   hand_irq:      .word intr_hang
   hand_fiq:      .word intr_hang
   
reset:
   /* copy vector data to 0x0000, see https://github.com/dwelch67/raspberrypi */
   mov r0,#0x8000
   mov r1,#0x0000
   ldmia r0!,{r2,r3,r4,r5,r6,r7,r8,r9}
   stmia r1!,{r2,r3,r4,r5,r6,r7,r8,r9}
   ldmia r0!,{r2,r3,r4,r5,r6,r7,r8,r9}
   stmia r1!,{r2,r3,r4,r5,r6,r7,r8,r9}   
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
   bl    program
h: bl h
   
   
intr_undef: 
   mrs   r0, cpsr
   mov   r1, r14
   bl    itr_undef

intr_hang: 
   mrs   r0, cpsr
   mov   r1, r14
   bl    itr_hang

