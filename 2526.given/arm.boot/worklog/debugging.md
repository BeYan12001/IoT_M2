
# Discussing Advanced Debugging

As you probably noticed already, you can debug using `gdb` in 
pretty much the same way as you would on a regular Linux machine.
All the usual commands do work, which is great.

A GDB Cheat Sheet for the basic commands is available [here](gdb.md).

## Breakpoints

Note however that the number of breakpoints is very limited
on real boards, often no more than 4 breakpoints. Why? 
Because they are hardware breakpoints and the processor only
supports a hand full. 

Hopefully, QEMU has more breakpoints than that.

## Checking Memory Size

Normally, a Versatile Board has 64MB of memory, but we want to play 
with much smaller footprint than that. The makefile configures the 
memory size to be 16KB by default. (check in the makefile). 

But is it enough? 

So let's check this. Please add this C function to the file `main.c`
before the C function `_start()`.

```c
void check_memory() {
  void *max = (void*)MEMORY;
  void *addr = &stack_top;
  if (addr >= max)
    panic();
}
```
Then call it right at the beginning of the function `_start`:

```c
void _start() {
  check_memory();
  ...
}
```

Let's single-step the execution with `gdb`:

- start a debug session
- set a breakpoint in `_start`
- step in the function `check_stacks`
- verify the addresses `max` and `addr`, printing them with `gdb`.

  ```gdb
  (gdb) p /x max
  $3 = 0x4000
  (gdb) p /x stack_top
  $4 = 0x0
  (gdb) p /x addr
  $5 = 0x2320
  ```
So our code, data, and stack already consume more than 8KB, 
with our section layout in our linker script. Please, check
the linker script.

You can also confirm it looking at our kernel files:

```bash
$ ls -alh build/versatile/kernel.*
-rwxrwxr-x 1 ogruber ogruber 4.8K Jan 18 18:01 build/versatile/kernel.bin
-rwxrwxr-x 1 ogruber ogruber  15K Jan 18 18:01 build/versatile/kernel.elf
```

**Question:** explain why the `kernel.elf` is larger than the `kernel.bin`

**Question:** explain the code in the function `check_memory`.

## Adding an embedded `printf`

Let's add some more code: an small embedded version of printf,
called `kprintf`, defined in the source `kprintf.c`.

**So please modify the makefile accordingly.**

Rebuild everything and check the kernel sizes again:

```bash
$ ls -alh build/versatile/kernel.*
-rwxrwxr-x 1 ogruber ogruber 12K Jan 18 18:12 build/versatile/kernel.bin
-rwxrwxr-x 1 ogruber ogruber 26K Jan 18 18:12 build/versatile/kernel.elf
```
As you can see, having a `prinft`, even an embedded version, is not adding a small amount of code...

Let's check again via our function `check_memory`:

```gdb
(gdb) p /x max
$1 = 0x4000
(gdb) p /x addr
$2 = 0x3d10
```
Confirmed. 

So you see, the way you configure memory in the makefile, the amount of code linked together, the amount of stack you allocate in the linked script, all these facets of your software must be aligned and consistent. 

## Faulty Addresses

One of the main difference when debugging an embedded system
is with respect to faulty addresses, either manipulating data 
or executing code. Invalid memory accesses 
do raise ***hardware exceptions***, making the execution flow
go through the hardware exception vector at the address 0x0000-0000.

```armasm
// Hardware Exception Vector (loaded 0x0000-0000, see linker script)
0x00     ldr pc, reset_handler_addr
0x04     ldr pc, undef_handler_addr
0x08     ldr pc, swi_handler_addr
0x0C     ldr pc, prefetch_abort_handler_addr
0x10     ldr pc, data_abort_handler_addr
0x14     ldr pc, unused_addr
0x18     ldr pc, irq_handler_addr
0x1C     ldr pc, fiq_handler_addr

0x20  reset_handler_addr: .word _reset_handler  
0x24  undef_handler_addr: .word _undef_handler
0x28  swi_handler_addr: .word _swi_handler
0x2C  prefetch_abort_handler_addr: .word _prefetch_abort_handler
0x30  data_abort_handler_addr: .word _data_abort_handler
0x34  not_used_addr: .word _not_used_handler
0x38  irq_handler_addr: .word _isr_handler
0x3C  fiq_handler_addr: .word _fiq_handler
```

When does that happen? 
- undefined instruction exception
  - the execution strays somewhere where it is not valid assembly code,
    but the address range is valid memory
  - the executed code has been trashed via faulty pointers
- prefetch abort
  - the execution strays in an invalid address range in memory
- data abort
  - a load or store operation tries to manipulate an invalid address in memory

On Linux, the operating system catches those exceptions and surface them through SEGV signal that GDB understands. 

**NOT WITH BARE METAL PROGRAMMING** 

To help you understand how to debug your code when it will do invalid memory accesses, we will provoke one voluntarily, a data abort exception to be precise.

So let's modify the given code slightly to force QEMU to actually raise a hardware data-abort exception.

First, in the file `startup.s`, we will turn on the hardware check
for misaligned memory access. Add the following three lines right 
before the upcall to the C function `_start`.

```armasm
MRC p15, 0, r0, c1, c0, 0
ORR r0, r0, #(1 << 1)   @ SCTLR.A
MCR p15, 0, r0, c1, c0, 0
```

The three lines of code is to force QEMU to raise an data-abort exception when executing the following C code:

```c
  volatile uint32_t *p = (uint32_t *)0xDEADBEEF;
  uint32_t x = *p;
```
So let's add this code right after the call to `check_memory`.

```c
37  void _start() {
38    check_memory();
39
40    volatile uint32_t *p = (uint32_t *)0xDEADBEEF;
41    uint32_t x = *p;
42
...
69  }
```

Rebuild everything and launch a debug session.

```gdb
(gdb) br _start
Breakpoint 1 at 0x10ec: file main.c, line 38.
(gdb) cont
Continuing.

Breakpoint 1, _start () at main.c:38
38	  check_memory();
(gdb) n
40	  volatile uint32_t *p = (uint32_t *)0xDEADBEEF;
(gdb) step
41	  uint32_t x = *p;
(gdb) next
?? () at exception.s:28
28	     ldr pc, data_abort_handler_addr
(gdb) stepi
_data_abort_handler () at exception.s:89
89	    MRC p15, 0, r0, c6, c0, 0   @ DFAR
```

Here you go, you just debugged through a hardware exception!

Keep single stepping until the infinite loop at the end of the function

```armasm
  1: b 1b // loop for debug
```

At that point, the processor registers contain the following
values:

    r0 = DFAR (faulting data address)
    r1 = DFSR (fault status)
    r2 = SPSR_abt
    r3 = faulting instruction address */

For now, we are interested mostly in `r0` and `r3`

```gdb
(gdb) p /x $r3
$1 = 0x1100
(gdb) p /x $r1
$2 = 0x1
(gdb) p /x $r0
$3 = 0xdeadbeef
```

You can recognize the value `0xdeadbeef`, it is the address we used to do an invalid memory access in the function `_start`. 

But what is the address `0x1100`, it is the address of the instruction whose execution cause the hardware exception to be raised. But which one is it?

We can use the gdb command `list` for that:

```gdb
(gdb) list *0x1100
0x1100 is in _start (main.c:41).
36	 */
37	void _start() {
38	  check_memory();
39	
40	  volatile uint32_t *p = (uint32_t *)0xDEADBEEF;
41	  uint32_t x = *p;
42	
43	  uart_send_string(UART0, "\nHello world!\n");
(gdb) 
```

We could also use:

```gdb
(gdb) info line *0x1100
Line 41 of "main.c" starts at address 0x10fc <_start+28> and ends at 0x1108 <_start+40>.
```

And then disassemble the code there, using either

    (gdb) disassemble _start
    Dump of assembler code for function _start:
    ...
    0x00001100 <+32>:	ldr	r3, [r3]
    ...
    End of assembler dump.

or 

    (gdb) disassemble 0x10fc
    Dump of assembler code for function _start:
    ...
    0x00001100 <+32>:	ldr	r3, [r3]
    ...
    End of assembler dump.

In both cases, you can see the offending load instruction
at address 0x1100.

For information, here is the full assembler dump in our case:

```armasm
   0x000010e0 <+0>:	push	{r11, lr}
   0x000010e4 <+4>:	add	r11, sp, #4
   0x000010e8 <+8>:	sub	sp, sp, #24
   0x000010ec <+12>:	bl	0x10a0 <check_memory>
   0x000010f0 <+16>:	movw	r3, #48879	@ 0xbeef
   0x000010f4 <+20>:	movt	r3, #57005	@ 0xdead
   0x000010f8 <+24>:	str	r3, [r11, #-8]
   0x000010fc <+28>:	ldr	r3, [r11, #-8]
   0x00001100 <+32>:	ldr	r3, [r3]
   0x00001104 <+36>:	str	r3, [r11, #-12]
   0x00001108 <+40>:	movw	r1, #11412	@ 0x2c94
   0x0000110c <+44>:	movt	r1, #0
   0x00001110 <+48>:	mov	r0, #4096	@ 0x1000
   0x00001114 <+52>:	movt	r0, #4127	@ 0x101f
   0x00001118 <+56>:	bl	0x1280 <uart_send_string>
   0x0000111c <+60>:	movw	r1, #11428	@ 0x2ca4
   0x00001120 <+64>:	movt	r1, #0
   0x00001124 <+68>:	mov	r0, #4096	@ 0x1000
   0x00001128 <+72>:	movt	r0, #4127	@ 0x101f
   0x0000112c <+76>:	bl	0x1280 <uart_send_string>
   0x00001130 <+80>:	mov	r3, #0
   0x00001134 <+84>:	str	r3, [r11, #-16]
   0x00001138 <+88>:	mov	r3, #0
   0x0000113c <+92>:	str	r3, [r11, #-20]	@ 0xffffffec
   0x00001140 <+96>:	sub	r3, r11, #21
   0x00001144 <+100>:	mov	r1, r3
   0x00001148 <+104>:	mov	r0, #4096	@ 0x1000
   0x0000114c <+108>:	movt	r0, #4127	@ 0x101f
   0x00001150 <+112>:	bl	0x11b0 <uart_receive>
   0x00001154 <+116>:	mov	r3, r0
   0x00001158 <+120>:	cmp	r3, #0
   0x0000115c <+124>:	beq	0x11a8 <_start+200>
   0x00001160 <+128>:	ldrb	r3, [r11, #-21]	@ 0xffffffeb
   0x00001164 <+132>:	cmp	r3, #13
   0x00001168 <+136>:	bne	0x1190 <_start+176>
   0x0000116c <+140>:	mov	r1, #13
   0x00001170 <+144>:	mov	r0, #4096	@ 0x1000
   0x00001174 <+148>:	movt	r0, #4127	@ 0x101f
   0x00001178 <+152>:	bl	0x121c <uart_send>
   0x0000117c <+156>:	mov	r1, #10
   0x00001180 <+160>:	mov	r0, #4096	@ 0x1000
   0x00001184 <+164>:	movt	r0, #4127	@ 0x101f
   0x00001188 <+168>:	bl	0x121c <uart_send>
   0x0000118c <+172>:	b	0x1140 <_start+96>
   0x00001190 <+176>:	ldrb	r3, [r11, #-21]	@ 0xffffffeb
   0x00001194 <+180>:	mov	r1, r3
   0x00001198 <+184>:	mov	r0, #4096	@ 0x1000
   0x0000119c <+188>:	movt	r0, #4127	@ 0x101f
   0x000011a0 <+192>:	bl	0x121c <uart_send>
   0x000011a4 <+196>:	b	0x1140 <_start+96>
   0x000011a8 <+200>:	nop	{0}
   0x000011ac <+204>:	b	0x1140 <_start+96>
```

<span style="color:red">IMPORTANT:</span> it will happen that
the execution will raise a hardware exception and it will get 
stuck in the infinite loop, with nothing happening anymore.

It is important for you to know what to do then.

So let's restart the execution... STOP: no need to kill everything!

```gdb
(gdb) set $r15=0
(gdb) cont
Continuing.

Breakpoint 1, _start () at main.c:38
(gdb) cont
Continuing.

```
The execution seems to be hung somewhere... and it is... in the infinite loop of the data-abort handler

    Use `ctrl-c` to regain control in the gdb console.

```gdb
^C
Program received signal SIGINT, Interrupt.
_data_abort_handler () at exception.s:113
113	 1: b 1b // loop for debug
(gdb) 
```
You are in the handler, the registers `r0` and `r3`
contain the addresses you are interested in. 
How do you know?

```gdb
(gdb) list
108	
109	    /* r0 = DFAR (faulting data address)
110	       r1 = DFSR (fault status)
111	       r2 = SPSR_abt
112	       r3 = faulting instruction address */
113	 1: b 1b // loop for debug
114	
115	.global _panic
116	   .func _panic
117	_panic:
(gdb) 
```

Reading the comment, you know and you can print
the relevant registers:

```gdb
(gdb) p /x $r0
$4 = 0xdeadbeef
(gdb) p /x $r3
$5 = 0x1100
```

And then look at the offending instruction, like we did before:

```gdb
(gdb) list *0x1100
0x1100 is in _start (main.c:41).
36	 */
37	void _start() {
38	  check_memory();
39	
40	  volatile uint32_t *p = (uint32_t *)0xDEADBEEF;
41	  uint32_t x = *p;
42	
(gdb) 
```

So that's it, you are now equipped to debug bare metal software.

Congratulations!

## Cleanup the code

Time to remove what we added to provoke the data-abort exception:

- comment out the three lines in `startup.s`
```armasm
// MRC p15, 0, r0, c1, c0, 0
// ORR r0, r0, #(1 << 1)   @ SCTLR.A
// MCR p15, 0, r0, c1, c0, 0
```
- comment out the two lines in `main.c`
```c
//  volatile uint32_t *p = (uint32_t *)0xDEADBEEF;
//  uint32_t x = *p;
```

We are back in business, the code works again.
