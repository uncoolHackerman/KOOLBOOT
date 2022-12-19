// FAT.h 03/12/2022 - 19/12/2022
// FAT12 and FAT16 Drivers for KOOLBOOT
// fork of the FAT12 Driver for COOLBOOT
// written by Gabriel Jickells

#ifndef _FAT_H_
#define _FAT_H_

#include <stdint.h>
#include "disk.h"
#include "memory.h"
#include "string.h"
#include <stddef.h>

typedef struct BootSector
{
    char Identifier[11];
    uint16_t BytesPerSector;
    uint8_t SectorsPerCluster;
    uint16_t ReservedSectors;
    uint8_t FatCount;
    uint16_t RootDirEntries;
    uint16_t TotalSectors;
    uint8_t MediaDescriptor;
    uint16_t SectorsPerFAT;
    uint16_t SectorsPerTrack;
    uint16_t HeadCount;
    uint32_t HiddenSectors;
    uint32_t LargeSectors;
    uint8_t DriveNumber;
    uint8_t WinNTFlags;
    uint8_t Signature;
    char SerialNumber[4];
    char VolumeID[11];
    char SystemID[8];
} __attribute__((packed)) BootSector;

typedef struct DirectoryEntry
{
    char Name[11];
    uint8_t Attributes;
    uint8_t Reserved;
    uint8_t CreationTimeTenths;
    uint16_t CreationTime;
    uint16_t CreationDate;
    uint16_t AccessedDate;
    uint16_t FirstClusterHigh;
    uint16_t ModificationTime;
    uint16_t ModificationDate;
    uint16_t FirstClusterLow;
    uint32_t Size;
} __attribute__((packed)) DirectoryEntry;

typedef struct FAT_Data
{
    union BootSect
    {
        BootSector u_BootSector;
        uint8_t BootSectorBytes[512];
    } BootSect;
} FAT_Data;

static FAT_Data g_FatData;
static uint8_t* g_FAT = NULL;

bool ReadBootRecord(DISK* disk, uint8_t Drive);
bool ReadRootDirectory(DISK* disk, uint8_t Drive, void* BufferOut);
DirectoryEntry* FindFile(DirectoryEntry* Dir, char* Name);
bool ReadFile(DISK* disk, uint8_t Drive, DirectoryEntry* file, void* BufferOut);
uint32_t Cluster2LBA(uint16_t Cluster);
bool ReadFat(DISK* disk, uint8_t Drive);
bool FatInitialise(DISK* disk, uint8_t Drive);
bool OpenDirectory(DISK* disk, uint8_t drive, char* path);

uint32_t g_DataSectionLBA = 33;             // the default for KOOLBOOT on a floppy
uint8_t FAT_VER = 12;                       // the default for KOOLBOOT on a floppy

bool ReadBootRecord(DISK* disk, uint8_t Drive)
{
    return ReadSectors(disk, Drive, 0, 1, &g_FatData.BootSect.BootSectorBytes);
}

bool ReadRootDirectory(DISK* disk, uint8_t Drive, void* BufferOut)
{
    uint32_t RootDirectoryLBA = g_FatData.BootSect.u_BootSector.ReservedSectors + (g_FatData.BootSect.u_BootSector.FatCount * g_FatData.BootSect.u_BootSector.SectorsPerFAT);
    uint32_t RootDirectorySize = g_FatData.BootSect.u_BootSector.RootDirEntries * sizeof(DirectoryEntry);
    uint32_t RootDirectorySectors = RootDirectorySize / g_FatData.BootSect.u_BootSector.BytesPerSector;
    if(RootDirectorySize % g_FatData.BootSect.u_BootSector.BytesPerSector) RootDirectorySectors++;
    g_DataSectionLBA = RootDirectoryLBA + RootDirectorySectors;
    return ReadSectors(disk, Drive, RootDirectoryLBA, RootDirectorySectors, BufferOut);
}

DirectoryEntry* FindFile(DirectoryEntry* Dir, char* Name) {
    char FatName[12] = "           ";
    char* extension = strchr(Name, '.');
    if(extension && *extension != 0) {
        *extension = 0;
        extension++;            // ignore the .
        for(int i = 0; i < strlen(extension); i++)
            FatName[i + 8] = ToUpper(extension[i]);
    }
    for(int i = 0; i < strlen(Name); i++) {
        if(!Name[i] || i >= 11) break;
        FatName[i] = ToUpper(Name[i]);
    }
    for(int i = 0; i < g_FatData.BootSect.u_BootSector.RootDirEntries; i++) {
        if(Dir[i].Name[0] == '\0') break;
        if(memcmp(Dir[i].Name, FatName, 11))
        return &Dir[i];
    }
    return NULL;
}

bool ReadFile(DISK* disk, uint8_t Drive, DirectoryEntry* file, void* BufferOut)
{
    uint16_t CurrentCluster = file->FirstClusterLow;
    uint32_t FatIndex;
    do
    {
        if(!ReadSectors(disk, Drive, Cluster2LBA(CurrentCluster), g_FatData.BootSect.u_BootSector.SectorsPerCluster, BufferOut)) return false;
        BufferOut += g_FatData.BootSect.u_BootSector.BytesPerSector * g_FatData.BootSect.u_BootSector.SectorsPerCluster;
        if(FAT_VER == 16) {
            FatIndex = CurrentCluster * 2;
            CurrentCluster = *(uint16_t*)(g_FAT + FatIndex);
        }
        else {
            FatIndex = CurrentCluster * 3 / 2;
            if(CurrentCluster & 1) CurrentCluster = *(uint16_t*)(g_FAT + FatIndex) >> 4;
            else CurrentCluster = *(uint16_t*)(g_FAT + FatIndex) & 0xFFF;
        }
    } while ((CurrentCluster < 0xFF8 && FAT_VER == 12) || (CurrentCluster < 0xFFF8 && FAT_VER == 16));
    return true;
}

uint32_t Cluster2LBA(uint16_t Cluster)
{
    return g_DataSectionLBA + (Cluster - 2) * g_FatData.BootSect.u_BootSector.SectorsPerCluster;
}

bool ReadFat(DISK* disk, uint8_t Drive)
{
    return ReadSectors(disk, Drive, g_FatData.BootSect.u_BootSector.ReservedSectors, g_FatData.BootSect.u_BootSector.SectorsPerFAT, g_FAT);
}

DirectoryEntry* g_CurrentDirectory = NULL;

bool FatInitialise(DISK* disk, uint8_t Drive) {
    if(!ReadBootRecord(disk, Drive)) {
        printf("Could not read boot record of drive 0%xh\n", Drive);
        return false;
    }
    g_FAT = (uint8_t*)malloc(g_FatData.BootSect.u_BootSector.BytesPerSector * g_FatData.BootSect.u_BootSector.SectorsPerFAT);
    if(!ReadFat(disk, Drive)) {
        printf("Could not read FAT\n");
        return false;
    }
    g_CurrentDirectory = (DirectoryEntry*)malloc(g_FatData.BootSect.u_BootSector.RootDirEntries * sizeof(DirectoryEntry));
    if(!OpenDirectory(disk, Drive, "/")) {
        printf("Could not open root directory\n");
        return false;
    };
    if(memcmp(g_FatData.BootSect.u_BootSector.SystemID, (void*)"FAT12   ", 8)) {   // detect whether the disk uses FAT12 or FAT16
        FAT_VER = 12;
        return true;
    }
    if(memcmp(g_FatData.BootSect.u_BootSector.SystemID, (void*)"FAT16   ", 8)) {   // detect whether the disk uses FAT12 or FAT16
        FAT_VER = 16;
        return true;
    }
    return false;
}

bool OpenDirectory(DISK* disk, uint8_t drive, char* path) {
    if(path[0] == '/')
        ReadRootDirectory(disk, drive, g_CurrentDirectory);
    if(path[1] == 0) return true;
    char* nextPath;
    DirectoryEntry* fd;
    do {
        path++;
        nextPath = strchr(path, '/');
        if(nextPath) *nextPath = 0;
        fd = FindFile(g_CurrentDirectory, path);
        if(!fd) return false;
        ReadFile(disk, drive, fd, g_CurrentDirectory);
        path = nextPath;
    } while(nextPath && path[1]);
    return true;
}

#endif