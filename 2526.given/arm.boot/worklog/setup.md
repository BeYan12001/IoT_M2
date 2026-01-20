# Setup

This document is about the given setup.

## Needed Linux Packages

You will need a cross-compilation toolchain for the ARM processor.
The GNU toolchain is in the following Debian packages:

 - `gcc-arm-none-eabi`
 - `binutils-arm-none-eabi`

You will need the GNU debugger as well, called `gdb`. 
It is possible you have the following package

- `gdb-arm-none-eabi`

But usually there is only a multi-architecture debugger,
in the package:

-`gdb-multiarch`.

You will need QEMU for ARM systems, it is available in the following
Debian package:

- `qemu-system-arm`

## Trying it out

You have been given an archive and you extracted its content somewhere.
In the extracted contents, there is a directory called `workspace`, 
with a project `arm.boot`. It is a C project with a makefile.

    $ cd workspace/arm.boot
    $ make clean all
    $ make run
    ...

    Hello world!

And then you can type something and it appears, as expected. 
The given code simply echoes whatever is typed.

But if you wait a few seconds... you will see:

    Zzzz....

You will look later in the code why is this happening.

For fun, try moving around the screen with the arrow keys
and keep typing away... Surprising, right? You can indeed
move the cursor around and type anywhere. 

The window has become an old-fashion terminal, it is no longer
controlled by the shell, it is controlled by the program running
in the illusion provided by QEMU.

Part of the first day work is to better understand that...
along with understanding how to build and debug embedded
software.

