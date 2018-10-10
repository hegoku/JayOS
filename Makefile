ENTERPOINT  = 0x7E00

ASM			= nasm
CC          = bin/bin/i386-elf-gcc
LD			= bin/bin/i386-elf-ld
CFLAGS      = -c 

TARGET		= boot.bin loader kernel k1

OBJS        = build/kernel.o build/start.o build/interrupt.o

all : clean everything image

everything : boot.bin loader kernel k1

clean :
	rm -f build/*

image: everything buildimg

buildimg:
	dd if=build/boot.bin of=a.img bs=512 count=1 conv=notrunc
	hdiutil attach -imagekey diskimage-class=CRawDiskImage a.img
	cp build/loader /Volumes/JayOS/
	cp build/k1 /Volumes/JayOS/kernel
	hdiutil detach /Volumes/JayOS/

boot.bin : boot/boot.asm include/fat12hdr.inc
	$(ASM) -o build/$@ $<

loader : boot/loader.asm include/fat12hdr.inc include/elf.inc include/func.inc include/pm.inc
	$(ASM) -o build/$@ $<

kernel : $(OBJS)
	$(LD) -s -Ttext $(ENTERPOINT) -m elf_i386 -o build/kernel $(OBJS)

build/kernel.o : kernel/kernel.asm include/func.inc include/pm.inc
	$(ASM) -f elf -o $@ $<

build/start.o : kernel/start.c kernel/kernel.h kernel/interrupt.h kernel/asm_global.h
	$(CC) $(CFLAGS) -o $@ $<

build/interrupt.o : kernel/interrupt.c kernel/interrupt.h kernel/asm_global.h
	$(CC) $(CFLAGS) -o $@ $<

build/k1.o : kernel/k1.asm include/pm.inc include/func.inc
	$(ASM) -f elf -o $@ $<

k1 : build/k1.o
	$(LD) -s -Ttext $(ENTERPOINT) -m elf_i386 -o build/k1 build/k1.o

