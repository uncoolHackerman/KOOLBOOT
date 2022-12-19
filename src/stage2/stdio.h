// stdio.h 18/12/2022 - 18/12/2022
// stdio.h implementation for KOOLBOOT
// based on the COOLBOOT implementation for stdio.h
// Written by Gabriel Jickells

#ifndef _STDIO_H_
#define _STDIO_H_

#include <stdint.h>
#include "memory.h"
#include <stdarg.h>
#include <stdbool.h>

/*
To-do:
- give printf support for format specifiers longer than 1 character
*/

#define CHAR_BUFFER (char*)(0xb8000)
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define VGA_COLOUR(b,f) ((b << 4) | f)

enum VGA_Colours {
    VGA_BLACK = 0x00,
    VGA_BLUE = 0x01,
    VGA_GREEN = 0x02,
    VGA_CYAN = 0x03,
    VGA_RED = 0x04,
    VGA_MAGENTA = 0x05,
    VGA_BROWN = 0x06,
    VGA_LIGHT_GREY = 0x07,
    VGA_GREY = 0x08,
    VGA_LIGHT_BLUE = 0x09,
    VGA_LIME = 0x0A,
    VGA_LIGHT_CYAN = 0x0B,
    VGA_LIGHT_RED = 0x0C,
    VGA_PINK = 0x0D,
    VGA_YELLOW = 0x0E,
    VGA_WHITE = 0x0F
};

uint32_t CursorX = 0;
uint32_t CursorY = 0;
uint8_t CharColour = VGA_COLOUR(VGA_BLACK, VGA_WHITE);

void putc(char c);
void ScrollScreen(uint32_t lines);
void puts(char* s);
void ClrScr(void);
void putnum(uint32_t num, int base, bool sign);
void printf(const char* fmt, ...);

void putc(char c) {
    char* Position = CHAR_BUFFER + ((CursorY * SCREEN_WIDTH + CursorX) << 1); // each entry in the char buffer is 2 bytes (character, colour)
    switch(c) {
        case '\n':                  // new line
            CursorY++;
            CursorX = 0;
            break;
        case '\r':                  // character return
            CursorX = 0;
            break;
        default:
            *Position = c;
            CursorX++;
            break;
    }
    Position++;
    *Position = CharColour;
    if(CursorX >= SCREEN_WIDTH) {   // see if we need to go to the next line
        CursorY++;
        CursorX = 0;
    }
    if(CursorY >= SCREEN_HEIGHT)    // see if we need to scroll the screen
        ScrollScreen(1);
    return;
}

void ScrollScreen(uint32_t lines) {
    CursorY -= lines;
    while(lines--) {
        for(uint32_t i = 0; i < (SCREEN_HEIGHT - 1); i++)
            memcpy(CHAR_BUFFER + (i * SCREEN_WIDTH * 2), CHAR_BUFFER + (i * SCREEN_WIDTH * 2 + SCREEN_WIDTH * 2), SCREEN_WIDTH * 2);
        memset(CHAR_BUFFER + ((SCREEN_HEIGHT - 1) * SCREEN_WIDTH * 2), 0, SCREEN_WIDTH * 2);
    }
    return;
}

void puts(char* s) {
    while(*s) {
        putc(*s);
        s++;
    }
    return;
}

void ClrScr(void) {
    for(int i = 0; i < SCREEN_HEIGHT * SCREEN_WIDTH * 2; i++)
        *(CHAR_BUFFER + i) = i & 1 ? CharColour : 0;
    CursorX = 0;
    CursorY = 0;
    return;
}

char g_Hex[] = "0123456789ABCDEF";

void putnum(uint32_t num, int base, bool sign)
{
    char Buffer[32] = {0};
    int i = 0;
    signed long oldnum = (signed long)num;
    if(sign && (signed long)num < 0) num = -num;
    uint32_t rem;
    do
    {
        rem = num % base;
        num /= base;
        Buffer[i] = g_Hex[rem];
        i++;
    } while(num);
    if(sign && oldnum < 0) Buffer[i] = '-';
    for(int j = 31; j >= 0; j--)
        if(Buffer[j])putc(Buffer[j]);
    return;
}

enum PrintfStates
{
    PRINTF_STATE_NORMAL = 0,
    PRINTF_STATE_TYPE
};

// very basic, needs to be made better to support format specifiers more than 1 byte (%hhx, %lu, etc...)
void printf(const char* fmt, ...)
{
    int state = PRINTF_STATE_NORMAL;
    va_list args;
    va_start(args, fmt);
    while(*fmt)
    {
        switch (state)
        {
            case PRINTF_STATE_TYPE:
                switch (*fmt)
                {
                    case 's':
                        puts(va_arg(args, char*));
                        break;
                    case '%':
                        putc('%');
                        break;
                    case 'c':
                        putc((char)va_arg(args, int));
                        break;
                    case 'u':
                        putnum(va_arg(args, unsigned int), 10, 0);
                        break;
                    case 'x':
                        putnum(va_arg(args, unsigned int), 16, 0);
                        break;
                    case 'i':
                        putnum(va_arg(args, int), 10, 1);
                        break;
                    case 'o':
                        putnum(va_arg(args, unsigned int), 8, 0);
                        break;
                    default:
                        break;
                }
                state = PRINTF_STATE_NORMAL;
                break;
            default:
                switch(*fmt)
                {
                    case '%':
                        state = PRINTF_STATE_TYPE;
                        break;
                    default:
                        putc(*fmt);
                        break;
                }
        }
        fmt++;
    }
    va_end(args);
    return;
}

#endif              // include guard