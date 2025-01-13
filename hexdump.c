#include <ctype.h>
#include <stdio.h>

#include "hexdump.h"

/**
 * Prints one line of hex bytes of data. If the data is shorter than the width, the rest of the line is filled with "--".
 * @param data Pointer to the begining of data.
 * @param start Index to print from.
 * @param length Length of the input data.
 * @param width Width of printed line in bytes.
 */
static void printRaw(char *data, size_t start, size_t length, int width) {
    for (size_t it = start; it < start + width; it++) {
        if (it < length) {
            printf("%02x", (unsigned int)data[it]);
        } else {
            printf("--");
        }

        if (it != start + width - 1) {
            printf(" ");
        }
    }
}

/**
 * Prints one line of string data. If the data is shorter than the width, the rest of the line is filled with "*".
 * @param data Pointer to the begining of data.
 * @param start Index to print from.
 * @param length Length of the input data.
 * @param width Width of printed line in bytes.
 */
static void printString(char *data, size_t start, size_t length, int width) {
    for (size_t it = start; it < start + width; it++) {
        if (it < length) {
            char c = data[it];
            if (isprint(c) != 0) {
                printf("%c", c);
            } else {
                printf(".");
            }
        } else {
            printf("*");
        }
    }
}

/**
 * Prints a hexdump of the input data.
 * @param data Pointer to the begining of data.
 * @param length Length of the input data.
 * @param width Width of printed line in bytes.
 */
void hexdump(void *data, size_t length, int width) {
    if (width < 2) {
        width = 2;
        printf("Width must be >=2.");
    }
    printf("addr   | data  ");
    for (int i = 0; i < width - 2; i++) {
        printf("   ");
    }
    printf("| string\n");
    printf("-------------------");
    for (int i = 0; i < width - 2; i++) {
        printf("----");
    }
    printf("\n");
    //      0x0000 | 00 00 | ..
    for (size_t it = 0; it < length; it += width) {
        printf("0x%04x | ", it);
        printRaw(data, it, length, width);
        printf(" | ");
        printString(data, it, length, width);
        printf("\n");
    }
}