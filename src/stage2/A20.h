// A20.h 28/11/2022
// written by Gabriel Jickells

#ifndef _A20_H_
#define _A20_H_
#include <stdbool.h>
#include "io.h"

bool TestA20()
{
    char tmpAddr = *(char*)0x107DFE;
    char tmpAddr1 = *(char*)0x007DFE;
    *(char*)0x107DFE = 0;
    *(char*)0x007DFE = 1;
    bool A20on = *(char*)0x007DFE != *(char*)0x107DFE;
    *(char*)0x107DFE = tmpAddr;
    *(char*)0x007DFE = tmpAddr1;
    return A20on;
}

void EnableA20(void)
{
    if(TestA20()) return;
    outb(0x92, inb(0x92) | 2);
    io_wait();
    return;
}

void DisableA20(void) {
    if(!TestA20()) return;
    outb(0x92, inb(0x92) & ~2);
    io_wait();
    return;
}

#endif