// config.h 18/12/2022 - 18/12/2022
// COOLBOOT config.h implementation adapted to work with KOOLBOOT
// uses CB23110512v0.0.09 syntax (:OPTION=value;)
// Written by Gabriel Jickells

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "memory.h"
#include "disk.h"
#include "FAT.h"
#include <stdint.h>
#include <stdbool.h>

#define CONFIG_SIGN "CB23110512v0.0.09"             // the signature specifies which version of the (C/K)OOLBOOT config syntax is being used

char* g_COOLBOOTSYS = (char*)NULL;                  // this should never be changed after the config has been initialised
char* g_COOLBOOTSYS_BAK = (char*)NULL;              // this is the actual buffer that will be used by GetOption
uint32_t g_COOLBOOTSIZE = 0;                        // this value is needed for when the BAK buffer is being reset

bool InitialiseConfig(DISK* disk, uint8_t Drive);
char* GetOption(char* option);

// assumes the FAT has already been initialised
bool InitialiseConfig(DISK* disk, uint8_t Drive) {
    OpenDirectory(disk, Drive, "/");                // we might not be in the root directory
    DirectoryEntry* fd = FindFile(g_CurrentDirectory, "koolboot.kcf");
    if(!fd) {
        printf("Could not find configuration file \"/koolboot.kcf\"\n");
        return false;
    }
    g_COOLBOOTSIZE = fd->Size + 1024;
    g_COOLBOOTSYS = (char*)malloc(g_COOLBOOTSIZE);
    if(!ReadFile(disk, Drive, fd, g_COOLBOOTSYS)) return false;
    g_COOLBOOTSYS_BAK = (char*)malloc(g_COOLBOOTSIZE);
    memcpy(g_COOLBOOTSYS_BAK, g_COOLBOOTSYS, g_COOLBOOTSIZE);
    char* SIGNATURE = GetOption("SIGNATURE");
    if(!SIGNATURE) {
        printf("A signature is required to be present in koolboot.kcf but is missing\n");
        return false;
    }
    if(!memcmp(SIGNATURE, CONFIG_SIGN, strlen(strlen(SIGNATURE) > strlen(CONFIG_SIGN) ? SIGNATURE : CONFIG_SIGN))) {
        printf("koolboot.kcf signature is not valid\n");
        printf("see documentation for more information\n");
        return false;
    }
    return true;
}

char* GetOption(char* option) {
    memcpy(g_COOLBOOTSYS_BAK, g_COOLBOOTSYS, g_COOLBOOTSIZE);   // reset the GetOption buffer
    char* OPTION = strstr(g_COOLBOOTSYS_BAK, option);
    while(*(OPTION - strlen(option) - 1) != ':' || *OPTION != '=') {
        OPTION = strstr(OPTION, option);
        if(!OPTION) {
        printf("GetOption(): Could not find option \"%s\"\n", option);
        return (char*)NULL;
    }
    }
    OPTION++;
    char* endl = strchr(OPTION, ';');
    if(!endl) {
        printf("GetOption(): Syntax error: expected ';' at end of option %s\n", option);
        return (char*)NULL;
    }
    *endl = 0;
    return OPTION;
}

#endif