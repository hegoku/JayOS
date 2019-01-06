# ;ENTERPOINT  = 0x7E00
# ENTERPOINT  = 0x100000
ENTERPOINT  = 0xC0100000

ASM			= nasm
CC          = bin/bin/i386-elf-gcc
LD			= bin/bin/i386-elf-ld
CFLAGS      = -c -fno-builtin -I include/

TARGET		= boot.bin loader kernel k1 hdboot.bin hdloader

OBJS        = build/kernel.o build/start.o build/interrupt.o build/global.o build/keyboard.o build/tty.o build/desc.o build/process.o build/system_call.o \
				build/assert.o build/stdlib.o build/unistd.o build/stdio.o build/string.o build/math.o build/fs.o \
				build/floppy.o build/hd.o build/dev.o build/rootfs.o build/ext2.o build/fat.o build/mm.o build/page.o build/list.o build/schedule.o \
				build/errno.o build/devfs.o build/malloc.o

all : clean everything image

everything : boot.bin loader kernel k1 hdboot.bin hdloader

clean :
	rm -f build/*

image: everything buildimg

buildimg:
	dd if=build/boot.bin of=a.img bs=512 count=1 conv=notrunc
	hdiutil attach -imagekey diskimage-class=CRawDiskImage a.img
	cp build/loader /Volumes/JayOS/
	cp build/kernel /Volumes/JayOS/kernel
	hdiutil detach /Volumes/JayOS/
	dd if=build/hdboot.bin of=c1.img bs=1 count=448 conv=notrunc  seek=5243454 skip=62
	hdiutil attach -imagekey diskimage-class=CRawDiskImage c1.img
	cp build/hdloader /Volumes/NO\ NAME/loader
	cp build/kernel /Volumes/NO\ NAME/kernel
	program/build.sh
	hdiutil detach /Volumes/NO\ NAME/
	rm -f c1.vdi
	VBoxManage convertfromraw --format VDI c1.img c1.vdi
	vboxmanage storageattach orange --storagectl "IDE Controller" --port 0 --device 0 --medium emptydrive
	vboxmanage closemedium  disk c1.vdi
	vboxmanage storageattach orange --storagectl "IDE Controller" --port 0 --device 0 --medium c1.vdi --type hdd

boot.bin : boot/boot.asm include/fat12hdr.inc
	$(ASM)  -o build/$@ $<

loader : boot/loader.asm include/fat12hdr.inc include/elf.inc include/func.inc include/pm.inc
	$(ASM) -o build/$@ $<

hdboot.bin : boot/hdboot.asm include/fat12hdr.inc
	$(ASM)  -o build/$@ $<

hdloader : boot/hdloader.asm include/fat12hdr.inc include/elf.inc include/func.inc include/pm.inc
	$(ASM) -o build/$@ $<

kernel : $(OBJS)
	$(LD) -s -Ttext $(ENTERPOINT) -m elf_i386 -o build/kernel $(OBJS)

build/kernel.o : kernel/kernel.asm include/func.inc include/pm.inc
	$(ASM) -f elf -o $@ $<

build/start.o : kernel/start.c kernel/kernel.h kernel/interrupt.h kernel/global.h \
				include/system/process.h include/unistd.h include/stdio.h kernel/hd.h include/string.h include/fcntl.h \
				include/system/rootfs.h fs/ext2/ext2.h fs/fat/fat.h include/system/mm.h
	$(CC) $(CFLAGS) -o $@ $<

build/interrupt.o : kernel/interrupt.c kernel/interrupt.h kernel/global.h include/system/desc.h
	$(CC) $(CFLAGS) -o $@ $<

build/desc.o : lib/desc.c include/system/desc.h
	$(CC) $(CFLAGS) -o $@ $<

build/k1.o : kernel/k1.asm include/pm.inc include/func.inc
	$(ASM) -f elf -o $@ $<

build/keyboard.o: kernel/keyboard.c kernel/keyboard.h kernel/keymap.h kernel/global.h kernel/tty.h
	$(CC) $(CFLAGS) -o $@ $<

build/tty.o: kernel/tty.c kernel/tty.h include/system/fs.h
	$(CC) $(CFLAGS) -o $@ $<

build/assert.o: lib/assert.c include/assert.h kernel/global.h
	$(CC) $(CFLAGS) -o $@ $<

build/stdlib.o: lib/stdlib.c include/stdlib.h
	$(CC) $(CFLAGS) -o $@ $<

build/unistd.o: lib/unistd.c include/unistd.h include/sys/types.h include/system/system_call.h
	$(CC) $(CFLAGS) -o $@ $<

build/stdio.o: lib/stdio.c include/stdarg.h include/unistd.h include/sys/types.h include/stdio.h include/string.h kernel/global.h
	$(CC) $(CFLAGS) -o $@ $<

build/string_asm.o : lib/string.asm
	$(ASM) -f elf -o $@ $<

build/string_c.o : lib/string.c include/string.h
	$(CC) $(CFLAGS) -o $@ $<

build/string.o : build/string_asm.o build/string_c.o
	$(LD) -r -m elf_i386 -o build/string.o build/string_asm.o build/string_c.o

build/math.o : lib/math.c include/math.h
	$(CC) $(CFLAGS) -o $@ $<

build/process.o: kernel/process.c include/system/process.h include/sys/types.h kernel/global.h
	$(CC) $(CFLAGS) -o $@ $<

build/floppy.o: kernel/fd.c kernel/fd.h kernel/global.h
	$(CC) $(CFLAGS) -o $@ $<

build/hd.o: kernel/hd.c kernel/hd.h include/string.h kernel/global.h include/unistd.h \
			include/system/process.h include/math.h include/system/dev.h include/system/fs.h
	$(CC) $(CFLAGS) -o $@ $<

build/global.o: kernel/global.c kernel/global.h include/stdlib.h kernel/kernel.h include/stdio.h
	$(CC) $(CFLAGS) -o $@ $<
k1 : build/k1.o
	$(LD) -s -Ttext $(ENTERPOINT) -m elf_i386 -o build/k1 build/k1.o

build/system_call.o: kernel/system_call.c include/system/system_call.h kernel/tty.h \
					include/sys/types.h include/system/fs.h kernel/kernel.h include/system/fs.h include/system/process.h
	$(CC) $(CFLAGS) -o $@ $<

build/fs.o: fs/fs.c include/system/fs.h kernel/hd.h include/sys/types.h include/system/mm.h \
			include/system/process.h include/system/desc.h kernel/kernel.h \
			kernel/interrupt.h include/system/system_call.h  kernel/tty.h kernel/global.h include/system/list.h
	$(CC) $(CFLAGS) -o $@ $<

build/dev.o: fs/dev.c include/system/dev.h include/system/fs.h include/string.h kernel/global.h include/sys/types.h include/system/list.h
	$(CC) $(CFLAGS) -o $@ $<

build/rootfs.o: fs/rootfs/super.c include/system/fs.h include/system/rootfs.h include/system/mm.h
	$(CC) $(CFLAGS) -o $@ $<

build/ext2.o: fs/ext2/ext2.c include/system/fs.h include/sys/types.h include/system/dev.h fs/ext2/ext2.h kernel/global.h include/string.h
	$(CC) $(CFLAGS) -o $@ $<

build/fat.o: fs/fat/fat.c include/system/fs.h include/sys/types.h include/system/dev.h fs/fat/fat.h kernel/global.h include/string.h include/stdio.h \
				include/system/mm.h include/system/list.h
	$(CC) $(CFLAGS) -o $@ $<

build/mm.o: mm/mm.c include/system/mm.h include/string.h include/sys/types.h kernel/global.h
	$(CC) $(CFLAGS) -o $@ $<

build/list.o: kernel/list.c include/system/list.h kernel/global.h
	$(CC) $(CFLAGS) -o $@ $<

build/page.o: mm/page.c include/system/mm.h include/system/page.h include/sys/types.h
	$(CC) $(CFLAGS) -o $@ $<

build/schedule.o: kernel/schedule.c
	$(CC) $(CFLAGS) -o $@ $<

build/errno.o: lib/errno.c
	$(CC) $(CFLAGS) -o $@ $<

build/devfs.o: fs/devfs/devfs.c
	$(CC) $(CFLAGS) -o $@ $<

build/malloc.o: mm/malloc.c
	$(CC) $(CFLAGS) -o $@ $<