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

@ For binary long division info see http://courses.cs.vt.edu/~cs1104/BuildingBlocks/divide.030.html

.section ".text"

.globl __aeabi_uidivmod
.globl __aeabi_uidiv

__aeabi_uidiv:
__aeabi_uidivmod:
   cmp  r0, r1
   bmi  div_less
   push { r4, r5 }
   mov  r2, #0
   clz  r4, r0         @ count leading zeros dividend
   clz  r5, r1         @ count leading zeros divisor
   subs r4, r5, r4     @ the required divisor shift
   mov  r1, r1, LSL r4 @ do shift the divisor
   adds r4, r4, #1     @ 1 iteration + 1 for every devisor left shift
div_loop:
   beq  div_doneloop
   mov  r2, r2, LSL #1 @ shift quotient 1 bit
   cmp  r0, r1         @ is numerator > divisor
   blo  div_next       @ if not, the quotient LSB remains 0, go to next iteration
   orr  r2, r2, #1     @ set quotient LSB to 1
   sub  r0, r0, r1     @ substract divisor from dividend 
div_next:
   mov  r1, r1, LSR #1 @ divisor 1 bit right shift
   subs r4, r4, #1     @ decrement iteration counter
   b    div_loop
   
div_doneloop:
   mov r1, r0
   mov r0, r2
   pop { r4, r5 }
   bx lr
   
div_less:
   mov r1, r0
   mov r0, #0
   bx lr
   
