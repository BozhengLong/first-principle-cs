#include "types.h"
#include "user.h"

// User-space library functions

void* memset(void *dst, int c, uint n) {
    char *d = (char*)dst;
    while (n-- > 0) {
        *d++ = c;
    }
    return dst;
}

int strlen(const char *s) {
    int n;
    for (n = 0; s[n]; n++)
        ;
    return n;
}

int strcmp(const char *p, const char *q) {
    while (*p && *p == *q) {
        p++;
        q++;
    }
    return (unsigned char)*p - (unsigned char)*q;
}

char* strcpy(char *s, const char *t) {
    char *os = s;
    while ((*s++ = *t++) != 0)
        ;
    return os;
}

// Simple printf for user programs
void printf(const char *fmt, ...) {
    // Simplified implementation
    // Real implementation would format and call write()
    write(1, fmt, strlen(fmt));
}

// Get a line of input
char* gets(char *buf, int max) {
    int i, cc;
    char c;

    for (i = 0; i+1 < max; ) {
        cc = read(0, &c, 1);
        if (cc < 1) {
            break;
        }
        buf[i++] = c;
        if (c == '\n' || c == '\r') {
            break;
        }
    }
    buf[i] = '\0';
    return buf;
}
