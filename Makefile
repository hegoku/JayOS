;ENTERPOINT  = 0x7E00
ENTERPOINT  = 0x100000

ASM			= nasm
CC          = bin/bin/i386-elf-gcc
LD			= bin/bin/i386-elf-ld
CFLAGS      = -c -fno-builtin -I include/

TARGET		= boot.bin loader kernel k1

OBJS        = build/kernel.o build/start.o build/interrupt.o build/global.o build/keyboard.o build/tty.o build/desc.o build/process.o \
				build/stdlib.o build/unistd.o build/stdio.o build/string.o \
				build/floppy.o build/hd.o

all : clean everything image

everything : boot.bin loader kernel k1

clean :
	rm -f build/*

image: everything buildimg

buildimg:
	dd if=build/boot.bin of=a.img bs=512 count=1 conv=notrunc
	hdiutil attach -imagekey diskimage-class=CRawDiskImage a.img
	cp build/loader /Volumes/JayOS/
	cp build/kernel /Volumes/JayOS/kernel
	hdiutil detach /Volumes/JayOS/

boot.bin : boot/boot.asm include/fat12hdr.inc
	$(ASM)  -o build/$@ $<

loader : boot/loader.asm include/fat12hdr.inc include/elf.inc include/func.inc include/pm.inc
	$(ASM) -o build/$@ $<

kernel : $(OBJS)
	$(LD) -s -Ttext $(ENTERPOINT) -m elf_i386 -o build/kernel $(OBJS)

build/kernel.o : kernel/kernel.asm include/func.inc include/pm.inc
	$(ASM) -f elf -o $@ $<

build/start.o : kernel/start.c kernel/kernel.h kernel/interrupt.h kernel/global.h kernel/process.h include/unistd.h include/stdio.h kernel/hd.h
	$(CC) $(CFLAGS) -o $@ $<

build/interrupt.o : kernel/interrupt.c kernel/interrupt.h kernel/global.h include/system/desc.h
	$(CC) $(CFLAGS) -o $@ $<

build/desc.o : lib/desc.c include/system/desc.h
	$(CC) $(CFLAGS) -o $@ $<

build/k1.o : kernel/k1.asm include/pm.inc include/func.inc
	$(ASM) -f elf -o $@ $<

build/keyboard.o: kernel/keyboard.c kernel/keyboard.h kernel/keymap.h kernel/global.h kernel/tty.h
	$(CC) $(CFLAGS) -o $@ $<

build/tty.o: kernel/tty.c kernel/tty.h
	$(CC) $(CFLAGS) -o $@ $<

build/stdlib.o: lib/stdlib.c include/stdlib.h
	$(CC) $(CFLAGS) -o $@ $<

build/unistd.o: lib/unistd.c include/unistd.h include/sys/types.h
	$(CC) $(CFLAGS) -o $@ $<

build/stdio.o: lib/stdio.c include/stdarg.h include/unistd.h include/sys/types.h include/stdio.h include/string.h kernel/global.h
	$(CC) $(CFLAGS) -o $@ $<

build/string_asm.o : lib/string.asm
	$(ASM) -f elf -o $@ $<

build/string_c.o : lib/string.c include/string.h
	$(CC) $(CFLAGS) -o $@ $<

build/string.o : build/string_asm.o build/string_c.o
	$(LD) -r -m elf_i386 -o build/string.o build/string_asm.o build/string_c.o

build/process.o: kernel/process.c kernel/process.h
	$(CC) $(CFLAGS) -o $@ $<

build/floppy.o: kernel/fd.c kernel/fd.h kernel/global.h
	$(CC) $(CFLAGS) -o $@ $<

build/hd.o: kernel/hd.c kernel/hd.h include/string.h kernel/global.h
	$(CC) $(CFLAGS) -o $@ $<

build/global.o: kernel/global.c kernel/global.h include/stdlib.h kernel/kernel.h include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<
k1 : build/k1.o
	$(LD) -s -Ttext $(ENTERPOINT) -m elf_i386 -o build/k1 build/k1.o

