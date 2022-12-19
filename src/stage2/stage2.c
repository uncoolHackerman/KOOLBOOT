// stage2.c 17/12/2022 - 19/12/2022
// main bootloader for KOOLBOOT
// based on COOLBOOT v0.0.13
// Written by Gabriel Jickells

#include "stdio.h"
#include "A20.h"
#include "disk.h"
#include "FAT.h"
#include "config.h"
#include <stdint.h>

#define KOOLBOOT_VER "0.0.01a"
#define KERNEL_START (void*)(0x20000)
#define KERNEL_CODE_SUCCESS 0l

char* g_kernel_errs[] = {
    "Success",                                  // 0x00000000, should never be shown
    "System Initialisation Failure",            // 0xFFFFFFFF, some vital system component couldn't be initialised
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

void main(uint8_t BootDrive) {
    ClrScr();                                                                               // reset the screen buffer to known values (empty)
    printf("KOOLBOOT Stage 2 v%s booted from drive 0%xh\n", KOOLBOOT_VER, BootDrive);
    printf("Enabling A20 Gate\n");                                                          // allows the operating system to use more than 1mb of RAM
    EnableA20();
    printf("Initialising disk 0%xh\n", BootDrive);                                          // so KOOLBOOT can read the kernel file from the disk
    DISK BootDisk;
    if(!DiskInitialise(&BootDisk, BootDrive)) {                                             // KOOLBOOT shouldn't continue execution if the disk initialisation fails
        CharColour = VGA_COLOUR(VGA_BLACK, VGA_RED);
        printf("Failed to initialise disk 0%xh\n", BootDrive);
        return;
    }
    printf("Disk 0%xh: %u Tracks, %u Sides, %u Sectors Per Track\n", BootDrive, BootDisk.Cylinders, BootDisk.Heads, BootDisk.SectorsPerCylinder);
    printf("Initialising FAT driver\n");                                                  // so KOOLBOOT can use FAT12 to find and read the kernel file
    if(!FatInitialise(&BootDisk, BootDrive)) {                                              // KOOLBOOT will not be able to load the kernel if the file system cannot be initialised
        CharColour = VGA_COLOUR(VGA_BLACK, VGA_RED);
        printf("Failed to initialise FAT Driver\n");
        return;
    }
    printf("FAT driver set up for FAT%u\n", FAT_VER);
    printf("Initialising system configuration\n");                                          // so KOOLBOOT knows where the kernel file is expected to be
    if(!InitialiseConfig(&BootDisk, BootDrive)) {                                           // KOOLBOOT will not know where the kernel is if the configuration could not be loaded
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
    char* KERNEL_FILE_NEXT = KERNEL_FILE;                                                   // stores the kernel path without the file name
    char* KERNEL_FILE_FINAL = KERNEL_FILE;                                                  // stores the file name in 8.3 format
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
    int ErrCode = StartKernel(BootDrive);                                                   // error code will be the return value of the kernel function
    __asm("cli");                                                                           // if the kernel has set up interrupts, they should be disabled when the kernel stops running
    ErrCode = -2;
    if(ErrCode == KERNEL_CODE_SUCCESS) return;                                              // don't clear the screen if there were no errors. Makes testing easier
    CharColour = VGA_COLOUR(VGA_RED, VGA_BLUE);
    ClrScr();
    printf("KOOLBOOT stage 2 v%s post-kernel environment\n", KOOLBOOT_VER);                 // now in KOOLBOOT post-kernel
    printf("Kernel program terminated with status 0x%x, (%i)\n", ErrCode, ErrCode);         // display the actual error code
    if(ErrCode <= 0 && ErrCode > -32) printf("ERROR: \"%s\"\n", g_kernel_errs[-ErrCode]);   // KOOLBOOT knows error codes from 0 to -31
    else printf("See documentation for more information\n");                                // unknown return values should be specified in kernel documentation
    return;
}