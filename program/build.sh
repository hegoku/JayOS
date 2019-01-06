#!/bin/bash
bin/bin/i386-elf-ar rcs program/build/libjnix.a build/system_call.o build/assert.o build/stdlib.o build/unistd.o build/stdio.o build/string.o build/math.o build/fs.o
#bin/bin/i386-elf-gcc -shared -fPIC -fno-builtin -Wall -o libjnix.so build/system_call.o build/assert.o build/stdlib.o build/unistd.o build/stdio.o build/string.o build/math.o build/fs.o

nasm -f elf -o program/build/start.o program/start.asm

bin/bin/i386-elf-gcc -I include/ -c -fno-builtin -Wall -o program/build/hello1.o program/hello.c
bin/bin/i386-elf-ld -o program/build/hello1 program/hello1.o program/build/start.o program/libjnix.a 

bin/bin/i386-elf-gcc -I include/ -c -fno-builtin -Wall -o program/build/shell.o program/shell.c
bin/bin/i386-elf-ld -o program/build/shell program/shell.o program/build/start.o program/libjnix.a 

bin/bin/i386-elf-gcc -I include/ -c -fno-builtin -Wall -o program/build/ls.o program/ls.c
bin/bin/i386-elf-ld -o program/build/ls program/ls.o program/build/start.o program/libjnix.a 

# hdiutil attach -imagekey diskimage-class=CRawDiskImage c1.img
cp program/build/hello1 /Volumes/NO\ NAME/
cp program/build/shell /Volumes/NO\ NAME/sh
cp program/build/ls /Volumes/NO\ NAME/ls
# hdiutil detach /Volumes/NO\ NAME/