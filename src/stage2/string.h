// string.h 04/12/2022
// written by Gabriel Jickells

#ifndef _STRING_H_
#define _STRING_H_

#include <stdint.h>

char* strchr(char* str, char chr);
uint32_t strlen(char* str);
char ToUpper(char c);
char* strstr(char* str, char* str2);

// a method for finding the first instance of the char chr in the string str
char* strchr(char* str, char chr) {
    while(*str) {
        if(*str == chr) return str;
        str++;
    }
    return NULL;        // chr is not in the string
}

uint32_t strlen(char* str) {
    uint32_t length = 0;
    while(*str) {
        length++;
        str++;
    }
    return length;
}

char ToUpper(char c) {
    if(c >= 'a' && c <= 'z') return c - 32;
    return c;
}

char ToLower(char c) {
    if(c >= 'A' && c <= 'Z') return c + 32;
    return c;
}

char* strstr(char* str, char* str2) {
    while(*str) {
        if(memcmp(str, str2, strlen(str2))) return str + strlen(str2);
        str++;
    }
    return NULL;
}

#endif