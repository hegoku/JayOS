#!/bin/bash
bin/bin/i386-elf-ar rcs program/crt.a build/system_call.o build/assert.o build/stdlib.o build/unistd.o build/stdio.o build/string.o build/math.o build/fs.o
nasm -f elf -o program/start.o program/start.asm
bin/bin/i386-elf-gcc -I include/ -c -fno-builtin -Wall -o program/hello1.o program/hello.c
bin/bin/i386-elf-ld -o program/hello1 program/hello1.o program/start.o program/crt.a 