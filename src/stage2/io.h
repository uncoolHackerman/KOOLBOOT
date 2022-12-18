// io.h 28/11/2022
// written by Gabriel Jickells

#ifndef _IO_H_
#define _IO_H_
#include <stdint.h>

extern uint16_t inb(uint16_t port);
extern void outb(uint16_t port, uint8_t data);
void io_wait(void);

void io_wait(void) {
    outb(0x80, 0);
    return;
}

#endif