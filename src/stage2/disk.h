// disk.h 29/11/2022
// written by Gabriel Jickells

#ifndef _DISK_H_
#define _DISK_H_

#include <stdint.h>
#include <stdbool.h>
#include "stdio.h"

/*
To-do:
- write a disk driver that will work without switching back to 16-bit real mode
*/

typedef struct DISK
{
    uint8_t DriveType;
    uint16_t Cylinders;
    uint8_t Heads;
    uint8_t SectorsPerCylinder;
} DISK;

extern bool BIOS_ReadSectors(uint8_t Drive, uint16_t Cylinder, uint8_t Head, uint8_t Sector, uint8_t Count, void* BufferOut);
extern bool BIOS_GetDriveParameters(uint8_t Drive, uint8_t* DriveType, uint16_t* Cylinders, uint16_t* Heads, uint16_t* Sectors);
extern bool BIOS_ResetDisk(uint8_t Drive);
bool ReadSectors(DISK* disk, uint8_t Drive, uint32_t lba, uint8_t Count, void* BufferOut);
void LBA2CHS(DISK* disk, uint32_t lba, uint16_t* Cylinder, uint16_t* Head, uint16_t* Sector);
bool DiskInitialise(DISK* disk, uint8_t Drive);

bool ReadSectors(DISK* disk, uint8_t Drive, uint32_t lba, uint8_t Count, void* BufferOut)
{
    uint16_t Cylinder, Head, Sector;
    LBA2CHS(disk, lba, &Cylinder, &Head, &Sector);
    for(int i = 0; i < 3; i++)
    {
        if(!BIOS_ReadSectors(Drive, Cylinder, Head, Sector, Count, BufferOut))
        {
            BIOS_ResetDisk(Drive);
            continue;
        }
        return true;
    }
    return false;
}

void LBA2CHS(DISK* disk, uint32_t lba, uint16_t* Cylinder, uint16_t* Head, uint16_t* Sector)
{
    *Cylinder = (lba / disk->SectorsPerCylinder) / disk->Heads;
    *Head = (lba / disk->SectorsPerCylinder) % disk->Heads;
    *Sector = (lba % disk->SectorsPerCylinder) + 1;
    return;
}

bool DiskInitialise(DISK* disk, uint8_t Drive)
{
    uint8_t DriveType;
    uint16_t Cylinders, Heads, Sectors;
    if(!BIOS_ResetDisk(Drive)) return false;
    if(!BIOS_GetDriveParameters(Drive, &DriveType, &Cylinders, &Heads, &Sectors)) return false;
    disk->DriveType = DriveType;
    disk->Cylinders = Cylinders;
    disk->Heads = Heads;
    disk->SectorsPerCylinder = Sectors;
    return true;
}

#endif