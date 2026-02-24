# GNU Debugger Cheat Sheet

The GNU toolchain provides a compiler and a debugger, called `gdb`.

Normally, the debugger `gdb` is tailored to the architecture of the Linux machine it runs on, which is probably based on an Intel/AMD processor. 

But here, we are cross-compiling and cross-executing on an embedded board, based on a ARM processor and without any operating system (bare-metal programming).

Therefore, we must a debugger for that architecture, based on an ARM processor. Two options exist:

- A dedicated debugger: `arm-none-eabi-gdb`
- A multi-architecture debugger: `gdb-multiarch`

We will use the later as it is the advocated way for both Ubuntu and Debian.

## Launch, Connect, and Disconnect

Assuming that you launched QEMU already, with the gdb-server accepting on port 1234 and having loaded the ELF file `kernel.elf`, you can launch gdb like this:

    $ gdb-multiarch kernel.elf
    ...
    (gdb) target remote localhost:1234
    Remote debugging using :1234
    ?? () at exception.s:24
    24	     ldr pc, reset_handler_addr
    (gdb) 

From there, it is pretty much a normal debug session.

**Notice:** you cannot use the commands `run` and `set args` as usual.

To end the debug session:

    (gdb) kill
    Kill the program being debugged? (y or n) y
    [Inferior 1 (process 1) killed]

You can quit the debugger like this:

    (gdb) quit
    $

## Command Help

For any gdb command, you can ask for the help:

    (gdb) help <command-name>

## Basic stepping commands

The commands are the following

- step/stepi
- next/nexti
- finish/continue

Using `gdb`, you can single step through C code and assembly code.

- Following C code:

      (gdb) step
      (gdb) next

- Following assembly code:

      (gdb) stepi
      (gdb) nexti

- Finish the current function, 
  returning to the caller function:

      (gdb) finish

- Resume the execution

      (gdb) continue


## Stack-related commands

The commands are the following

- where
- up/down
- call

You can ask where the execution is:

    (gdb) where

You can walk up and down the execution stack:

    (gdb) up
    (gdb) down

You can call a function

    (gdb) call foo(2,3)

## Printing-related Commands

You can print visible variables and arguments;

    (gdb) print <any C expression>

A very useful form to print memory addresses
or hexadecimal values:

    (gdb) print /x <any c expression>

Command to examine memory:

    (gdb) x /FMT <any C expression>

FMT is a repeat count followed by a format letter and a size letter.

- Format letters are o(octal), x(hex), d(decimal), u(unsigned decimal),
  t(binary), f(float), a(address), i(instruction), c(char), s(string)
  and z(hex, zero padded on the left).

- Size letters are b(byte), h(halfword), w(word), g(giant, 8 bytes).

The specified number of objects of the specified size are printed
according to the format.  If a negative number is specified, memory is
examined backward from the address.

## Breakpoint-related commands

- break <location>
- watch <expression>
- info breakpoint
- delete <no>

Setting a breakpoint:

    (gdb) break main
    (gdb) break hw.c:10

Setting a watchpoint:

    (gdb) watch <expression>

Listing breakpoints and watchpoints

    (gdb) info breakpoint

Deleting a breakpoint or watchpoint 
by its unique numéro:

    (gdb) delete <no>

Conditional breakpoints, an example:

```c
21 int _strlen(char *s) {
22   int count=0;
23   while (*s != '\0') { 
24     s = s + 1;
25     count = count+1;
26   }
27   return count;
28 }
```


    (gdb) br string.c:24
    Breakpoint 4 at string.c:24
    (gdb) condition 4 (*s=='A')
    ...
    (gdb) condition 4
    Breakpoint 4 now unconditional.
    (gdb)

# Window Layouts

You can also use different layouts of the `gdb` window. 
You can cycle through the different layouts 
using the `gdb` command 

    (gdb) layout next

You can also see the different possible layouts, like this:

    (gdb) layout
    List of tui layout subcommands:

    tui layout asm -- Apply the "asm" layout.
    tui layout next -- Apply the next TUI layout.
    tui layout prev -- Apply the previous TUI layout.
    tui layout regs -- Apply the TUI register layout.
    tui layout split -- Apply the "split" layout.
    tui layout src -- Apply the "src" layout.

<span style="color:red">IMPORTANT:</span> if you use layouts, be sure
to remember the way to restore a normal console layout: 
<span style="color:red">ctrl-x a</span> 

It is actually a toggle command, going back and forth between a normal console layout and the last selected `tui layout`
