# mcc : mutax's C Compiler
mcc is a simple C Compiler created with the goal self-hosting(compiler compile compiler own). mcc compile C program and output assembler for x86-64 witch is written by Intel-syntax.mcc support only c99.

This project my learning project that C Language and low-layer programing.This project reference to C Compiler book is written by Rui Ueyama @rui314.

*mcc* means ***m**utax's* ***C*** ***C**ompiler*. 

And last, I thanks a lot to @rui314 author C Compiler book.

# Build
build mcc.
``` Shell
make
```

clean modules.
``` Shell
make clean
```
# Usage
now mcc output only `.asm` file.
so when compile C-file, you need two step.
1. compile C-file to .asm file.
2. assemble .asm file with `cc` command or another assembler.
``` Shell
mcc source.c > out.asm
cc -o a.out out.asm
```


# Links to reference
[C Compiler Book](https://www.sigbus.info/compilerbook)

[ISO/IEC 9899:TC3](https://port70.net/~nsz/c/c99/n1256.html)
