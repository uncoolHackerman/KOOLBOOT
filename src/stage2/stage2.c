// stage2.c 17/12/2022 - 18/12/2022
// main bootloader for KOOLBOOT
// based on COOLBOOT
// Written by Gabriel Jickells

#include "stdio.h"
#include "A20.h"
#include "disk.h"
#include "FAT.h"
#include "config.h"

#define KOOLBOOT_VER "0.0.01"
#define KERNEL_START (void*)(0x20000)
#define KERNEL_CODE_SUCCESS 0l

void main(unsigned char BootDrive) {
    ClrScr();
    printf("KOOLBOOT Stage 2 v%s booted from drive 0%xh\n", KOOLBOOT_VER, BootDrive);
    printf("Enabling A20 Gate\n");
    EnableA20();
    printf("Initialising disk 0%xh\n", BootDrive);
    DISK BootDisk;
    if(!DiskInitialise(&BootDisk, BootDrive)) {
        CharColour = VGA_COLOUR(VGA_BLACK, VGA_RED);
        printf("Failed to initialise disk 0%xh\n", BootDrive);
        return;
    }
    printf("Disk 0%xh: %u Tracks, %u Sides, %u Sectors Per Track\n", BootDrive, BootDisk.Cylinders, BootDisk.Heads, BootDisk.SectorsPerCylinder);
    printf("Initialising FAT12 driver\n");
    if(!FatInitialise(&BootDisk, BootDrive)) {
        CharColour = VGA_COLOUR(VGA_BLACK, VGA_RED);
        printf("Failed to initialise FAT12 Driver\n");
        return;
    }
    printf("Root address: 0x%x\n", g_CurrentDirectory);
    printf("Fat address: 0x%x\n", g_FAT);
    printf("Initialising system configuration\n");
    if(!InitialiseConfig(&BootDisk, BootDrive)) {
        CharColour = VGA_COLOUR(VGA_BLACK, VGA_RED);
        printf("Failed to obtain system configuration\n");
        return;
    };
    char* KERNEL_FILE = GetOption("KERNEL_FILE");
    if(!KERNEL_FILE) {
        CharColour = VGA_COLOUR(VGA_BLACK, VGA_RED);
        printf("System configuration does not contain a kernel\n");
        return;
    }
    char* KERNEL_FILE_NEXT = KERNEL_FILE;
    char* KERNEL_FILE_FINAL = KERNEL_FILE;
    char tmpf;
    for(;;) {
        KERNEL_FILE_NEXT = strchr(KERNEL_FILE_FINAL, '/');
        if(!KERNEL_FILE_NEXT) {
            tmpf = *KERNEL_FILE_FINAL;
            *KERNEL_FILE_FINAL = 0;
            break;
        }
        KERNEL_FILE_NEXT++;
        KERNEL_FILE_FINAL = KERNEL_FILE_NEXT;
    }
    if(!OpenDirectory(&BootDisk, BootDrive, KERNEL_FILE)) {
        CharColour = VGA_COLOUR(VGA_BLACK, VGA_RED);
        printf("Could not find kernel directory, please update koolboot.kcf\n");
        return;
    }
    *KERNEL_FILE_FINAL = tmpf;
    DirectoryEntry* fd = FindFile(g_CurrentDirectory, KERNEL_FILE_FINAL);
    if(!fd) {
        CharColour = VGA_COLOUR(VGA_BLACK, VGA_RED);
        printf("Could not find kernel file\n");
        return;
    }
    ReadFile(&BootDisk, BootDrive, fd, KERNEL_START);
    int (*StartKernel)(uint8_t) = KERNEL_START;
    int ErrCode = StartKernel(BootDrive);
    __asm("cli");
    if(ErrCode == KERNEL_CODE_SUCCESS) return;
    ClrScr();
    CharColour = VGA_COLOUR(VGA_WHITE, VGA_RED);
    printf("KOOLBOOT stage 2 v%s post-kernel environment\n", KOOLBOOT_VER);
    printf("Kernel program terminated with status 0x%x\n", ErrCode);
    printf("See documentation for more information\n");
    return;
}