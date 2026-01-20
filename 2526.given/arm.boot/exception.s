/*
 * exception.s
 *
 *  Created on: Jan 24, 2021
 *      Author: ogruber
 */

 /* Standard definitions of Mode bits and Interrupt (I & F) flags in PSRs */

    .equ    CPSR_USR_MODE,       0x10
    .equ    CPSR_FIQ_MODE,       0x11
    .equ    CPSR_IRQ_MODE,       0x12
    .equ    CPSR_SVC_MODE,       0x13
    .equ    CPSR_ABT_MODE,       0x17
    .equ    CPSR_UND_MODE,       0x1B
    .equ    CPSR_SYS_MODE,       0x1F

    .equ    CPSR_IRQ_FLAG,         0x80      /* when set, IRQs are disabled, at the core level */
    .equ    CPSR_FIQ_FLAG,         0x40      /* when set, FIQs are disabled, at the core level */

 /* Exception Vector
  * assume this is linked and loaded at 0x0000-0000
  */
     ldr pc, reset_handler_addr
     ldr pc, undef_handler_addr
     ldr pc, swi_handler_addr
     ldr pc, prefetch_abort_handler_addr
     ldr pc, data_abort_handler_addr
     ldr pc, unused_handler_addr
     ldr pc, irq_handler_addr
     ldr pc, fiq_handler_addr

reset_handler_addr: .word _reset_handler
undef_handler_addr: .word _undef_handler
swi_handler_addr: .word _swi_handler
prefetch_abort_handler_addr: .word _prefetch_abort_handler
data_abort_handler_addr: .word _data_abort_handler
unused_handler_addr: .word _unused_handler
irq_handler_addr: .word _isr_handler
fiq_handler_addr: .word _fiq_handler

_isr_handler:
    b _isr_handler

_unused_handler:
    b _unused_handler // unused interrupt occurred

_fiq_handler:
	b _fiq_handler // unexpected fast interrupt

_swi_handler:
	b _swi_handler  // unexpected software interrupt

_undef_handler: // undefined instruction

    /* LR_und contains return address */
    MOV r1, lr

    /* Adjust to faulting instruction */
    MRS r0, SPSR
    TST r0, #0x20          @ Thumb?
    SUBEQ r1, r1, #4       @ ARM
    SUBNE r1, r1, #2       @ Thumb

    /* 
     * r1 = address of undefined instruction 
     */
 1: b 1b // loop for debug

_prefetch_abort_handler:
    MRC p15, 0, r0, c6, c0, 2   @ IFAR
    MRC p15, 0, r1, c5, c0, 1   @ IFSR
    MRS r2, SPSR
    MOV r3, lr

    /* Adjust PC depending on ARM/Thumb */
    TST r2, #0x20
    SUBEQ r3, r3, #4
    SUBNE r3, r3, #2

    /* r0 = IFAR
       r1 = IFSR
       r2 = SPSR_abt
       r3 = faulting instruction address */
 1: b 1b // loop for debug

_data_abort_handler:
  /* Read fault address */
    MRC p15, 0, r0, c6, c0, 0   @ DFAR

    /* Read fault status */
    MRC p15, 0, r1, c5, c0, 0   @ DFSR

    /* Read saved CPSR */
    MRS r2, SPSR

    /* LR_abt contains return address */
    MOV r3, lr

    /* Determine ARM vs Thumb, adjust to faulting instruction
     * Yes, the adjustment is different here than for prefetch aborts
     * or undefined instruction, why? Because the pipeline has fully
     * advanced (arm: 8 bytes, thumb: only 4 bytes).
    */
    TST r2, #0x20              @ Check T bit
    SUBEQ r3, r3, #8           @ ARM state
    SUBNE r3, r3, #4           @ Thumb state

    /* r0 = DFAR (faulting data address)
       r1 = DFSR (fault status)
       r2 = SPSR_abt
       r3 = faulting instruction address */
 1: b 1b // loop for debug

.global _panic
   .func _panic
_panic:
	b _panic
    .size _panic, . - _panic
    .endfunc
