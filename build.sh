#! /bin/bash
nasm boot/boot.asm -o build/boot.bin
dd if=build/boot.bin of=a.img bs=512 count=1 conv=notrunc
nasm boot/loader.asm -o build/loader
#nasm kernel.asm -o kernel
nasm -f elf kernel/kernel.asm -o lib/kernel.o
bin/bin/i386-elf-gcc -c -o lib/start.o kernel/start.c
bin/bin/i386-elf-ld -s -o build/kernel lib/kernel.o lib/start.o -m elf_i386 -Ttext 0x7E00
# bin/bin/i386-elf-ld -s -o build/kernel lib/kernel.o -m elf_i386 -Ttext 0x7E00
