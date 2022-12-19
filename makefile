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

VERSION=0.0.01a
FLOPPY_NAME=KOOLBOOT_v$(VERSION)_floppy.img
HDD_NAME=KOOLBOOT_v$(VERSION)_hdd.img

BINDIR=bin
SRCDIR=src

all: all_floppy clean all_hdd

all_floppy: dirs disk_floppy clean

all_hdd: dirs disk_hdd

# puts all of the components into a single disk image
disk_floppy: bootloader_floppy
	dd if=/dev/zero of=$(BINDIR)/$(FLOPPY_NAME) bs=512 count=2880
	dd if=$(BINDIR)/boot.bin of=$(BINDIR)/$(FLOPPY_NAME) conv=notrunc
	mcopy -i $(BINDIR)/$(FLOPPY_NAME) $(BINDIR)/koolboot.bin "::/koolboot.bin"
	mattrib -i $(BINDIR)/$(FLOPPY_NAME) +s +h "::/koolboot.bin"
	mcopy -i $(BINDIR)/$(FLOPPY_NAME) $(SRCDIR)/stage2/koolboot.kcf "::/koolboot.kcf"
	mattrib -i $(BINDIR)/$(FLOPPY_NAME) +s "::/koolboot.kcf"

disk_hdd: bootloader_hdd
	dd if=/dev/zero of=$(BINDIR)/$(HDD_NAME) bs=512 count=40960
	dd if=$(BINDIR)/boot.bin of=$(BINDIR)/$(HDD_NAME) conv=notrunc
	mcopy -i $(BINDIR)/$(HDD_NAME) $(BINDIR)/koolboot.bin "::/koolboot.bin"
	mattrib -i $(BINDIR)/$(HDD_NAME) +s +h "::/koolboot.bin"
	mcopy -i $(BINDIR)/$(HDD_NAME) $(SRCDIR)/stage2/koolboot.kcf "::/koolboot.kcf"
	mattrib -i $(BINDIR)/$(HDD_NAME) +s "::/koolboot.kcf"
	rm $(BINDIR)/*.bin
	rm $(BINDIR)/*.o

# makes sure there is a directory to compile into
dirs:
	mkdir -p $(BINDIR)
	rm -rf $(BINDIR)
	mkdir -p $(BINDIR)

# compiles and assembles the bootloader
bootloader_floppy:
	$(ASM32) $(SRCDIR)/boot_floppy.s -o $(BINDIR)/boot.o
	$(LD32) -Ttext 0x7C00 -e 0x7c00 --oformat binary $(BINDIR)/boot.o -o $(BINDIR)/boot.bin
	$(ASM32) $(SRCDIR)/stage2/stage2.s -o $(BINDIR)/stage2s.o
	$(CC32) $(CFLAGS32) $(SRCDIR)/stage2/stage2.c -o $(BINDIR)/stage2c.o
	$(CC32) $(LDFLAGS32) -T $(SRCDIR)/stage2/stage2.ld -Wl,-Map=$(BINDIR)/stage2.map $(BINDIR)/stage2s.o $(BINDIR)/stage2c.o -o $(BINDIR)/koolboot.bin $(CLIBS32)

bootloader_hdd:
	$(ASM32) $(SRCDIR)/boot_HDD.s -o $(BINDIR)/boot.o
	$(LD32) -Ttext 0x7C00 -e 0x7c00 --oformat binary $(BINDIR)/boot.o -o $(BINDIR)/boot.bin
	$(ASM32) $(SRCDIR)/stage2/stage2.s -o $(BINDIR)/stage2s.o
	$(CC32) $(CFLAGS32) $(SRCDIR)/stage2/stage2.c -o $(BINDIR)/stage2c.o
	$(CC32) $(LDFLAGS32) -T $(SRCDIR)/stage2/stage2.ld -Wl,-Map=$(BINDIR)/stage2.map $(BINDIR)/stage2s.o $(BINDIR)/stage2c.o -o $(BINDIR)/koolboot.bin $(CLIBS32)

# starts the emulator with the floppy disk containing KOOLBOOT for testing
run_floppy:
	qemu-system-i386 -drive file=$(BINDIR)/$(FLOPPY_NAME),if=floppy,format=raw

# starts the emulator with the hard disk containing KOOLBOOT for testing
run_hdd:
	qemu-system-i386 -drive file=$(BINDIR)/$(HDD_NAME),if=ide,format=raw

# gets rid of binary files that are no longer needed
clean:
	rm $(BINDIR)/*.bin
	rm $(BINDIR)/*.o