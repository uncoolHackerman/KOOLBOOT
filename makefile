# makefile 17/12/2022 - 18/12/2022
# makefile for building and running KOOLBOOT
# Written by Gabriel Jickells

ASM64=as
LD64=ld

CC32=/usr/local/i686-elf-gcc/bin/i686-elf-gcc
LD32=/usr/local/i686-elf-gcc/bin/i686-elf-ld
ASM32=/usr/local/i686-elf-gcc/bin/i686-elf-as

CFLAGS32=-c -O2 -ffreestanding -nostdlib
LDFLAGS32=-nostdlib
CLIBS32=-lgcc

VERSION=0.0.01
DISK_NAME=KOOLBOOT_v$(VERSION).img

BINDIR=bin
SRCDIR=src

all: disk clean

disk: dirs bootloader
	dd if=/dev/zero of=$(BINDIR)/$(DISK_NAME) bs=512 count=2880
	dd if=$(BINDIR)/boot.bin of=$(BINDIR)/$(DISK_NAME) conv=notrunc
	mcopy -i $(BINDIR)/$(DISK_NAME) $(BINDIR)/koolboot.bin "::/koolboot.bin"
	mattrib -i $(BINDIR)/$(DISK_NAME) +s +h "::/koolboot.bin"
	mcopy -i $(BINDIR)/$(DISK_NAME) $(SRCDIR)/stage2/koolboot.kcf "::/koolboot.kcf"
	mattrib -i $(BINDIR)/$(DISK_NAME) +s "::/koolboot.kcf"
	mcopy -i $(BINDIR)/$(DISK_NAME) $(BINDIR)/kernel.bin "::/kernel.bin"

dirs:
	mkdir -p $(BINDIR)
	rm -rf $(BINDIR)
	mkdir -p $(BINDIR)

bootloader:
	$(ASM32) $(SRCDIR)/boot.s -o $(BINDIR)/boot.o
	$(LD32) -Ttext 0x7C00 -e 0x7c00 --oformat binary $(BINDIR)/boot.o -o $(BINDIR)/boot.bin
	$(ASM32) $(SRCDIR)/stage2/stage2.s -o $(BINDIR)/stage2s.o
	$(CC32) $(CFLAGS32) $(SRCDIR)/stage2/stage2.c -o $(BINDIR)/stage2c.o
	$(CC32) $(LDFLAGS32) -T $(SRCDIR)/stage2/stage2.ld -Wl,-Map=$(BINDIR)/stage2.map $(BINDIR)/stage2s.o $(BINDIR)/stage2c.o -o $(BINDIR)/koolboot.bin $(CLIBS32)
	$(ASM32) $(SRCDIR)/kernel.s -o $(BINDIR)/kernel.o
	$(LD32) -Ttext 0x20000 -e 0x20000 --oformat binary $(BINDIR)/kernel.o -o $(BINDIR)/kernel.bin

run:
	qemu-system-i386 -drive file=$(BINDIR)/$(DISK_NAME),if=floppy,format=raw

clean:
	rm $(BINDIR)/*.bin
	rm $(BINDIR)/*.o