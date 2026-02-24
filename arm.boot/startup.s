 /* Standard definitions of Mode bits and Interrupt (I & F) flags in PSRs */

    .equ    CPSR_USR_MODE,       0x10
    .equ    CPSR_FIQ_MODE,       0x11
    .equ    CPSR_IRQ_MODE,       0x12
    .equ    CPSR_SVC_MODE,       0x13
    .equ    CPSR_ABT_MODE,       0x17
    .equ    CPSR_UND_MODE,       0x1B
    .equ    CPSR_SYS_MODE,       0x1F

    .equ    CPSR_IRQ_FLAG,         0x80      /* when set, IRQs are disabled, at the core. */
    .equ    CPSR_FIQ_FLAG,         0x40      /* when set, FIQs are disabled, at the core. */

.global _reset_handler
_reset_handler:
   /*
    * Set the core in the SYS_MODE, with all interrupts disabled,
    * and set the stack for the SYS_MODE
    */
	msr     cpsr_c,#(CPSR_SYS_MODE | CPSR_IRQ_FLAG | CPSR_FIQ_FLAG)
	ldr     sp,=stack_top             /* set the C stack pointer */

	/*-------------------------------------------
	 * Clear out the bss section, located from _bss_start to _bss_end.
	 * This is a C convention, the GNU GCC compiler will group
	 * all global variables that need to be zeroed on startup
	 * in the bss section of the ELF.
	 *-------------------------------------------*/
.clear:
	ldr	r4, =_bss_start
	ldr	r9, =_bss_end
	mov	r5, #0
1:
	stmia	r4!, {r5} 
	cmp	r4, r9
	blo	1b
  
 	/*
 	 * Now upcall the C entry function  _start(void)
 	 */
.upcall:
 	ldr r3,=_start
 	mov pc,r3

_halt:
	b	_halt

