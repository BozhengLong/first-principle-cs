#include "types.h"
#include "defs.h"
#include "x86.h"

// Simple console output
static void consputc(int c) {
    // Output to serial port (COM1)
    // Wait for transmit buffer to be empty
    while ((inb(0x3FD) & 0x20) == 0)
        ;
    outb(0x3F8, c);

    // Also output to VGA text mode (0xB8000)
    static uint16_t *crt = (uint16_t*)0xB8000;
    static int pos = 0;

    if (c == '\n') {
        pos += 80 - (pos % 80);
    } else {
        crt[pos++] = (c & 0xFF) | 0x0700;  // White on black
    }

    if (pos >= 80 * 25) {
        // Scroll
        memmove(crt, crt + 80, sizeof(crt[0]) * 80 * 24);
        pos -= 80;
        memset(crt + pos, 0, sizeof(crt[0]) * (80 * 25 - pos));
    }
}

// Print a string
static void printstr(const char *s) {
    while (*s) {
        consputc(*s++);
    }
}

// Print an integer
static void printint(int xx, int base, int sgn) {
    static char digits[] = "0123456789ABCDEF";
    char buf[16];
    int i, neg;
    uint x;

    neg = 0;
    if (sgn && xx < 0) {
        neg = 1;
        x = -xx;
    } else {
        x = xx;
    }

    i = 0;
    do {
        buf[i++] = digits[x % base];
    } while ((x /= base) != 0);

    if (neg) {
        buf[i++] = '-';
    }

    while (--i >= 0) {
        consputc(buf[i]);
    }
}

// Print a pointer
static void printptr(uint x) {
    int i;
    consputc('0');
    consputc('x');
    for (i = 0; i < (sizeof(uint) * 2); i++, x <<= 4) {
        consputc("0123456789ABCDEF"[x >> (sizeof(uint) * 8 - 4)]);
    }
}

// Simple printf implementation
void printf(const char *fmt, ...) {
    int *argp = (int*)(void*)(&fmt + 1);
    int c;

    for (int i = 0; (c = fmt[i] & 0xFF) != 0; i++) {
        if (c != '%') {
            consputc(c);
            continue;
        }
        c = fmt[++i] & 0xFF;
        if (c == 0) {
            break;
        }
        switch (c) {
        case 'd':
            printint(*argp++, 10, 1);
            break;
        case 'x':
        case 'p':
            printptr(*argp++);
            break;
        case 's':
            printstr((char*)*argp++);
            break;
        case '%':
            consputc('%');
            break;
        default:
            consputc('%');
            consputc(c);
            break;
        }
    }
}

// Panic: print message and halt
void panic(const char *s) {
    printf("PANIC: %s\n", s);
    while (1) {
        asm volatile("hlt");
    }
}
