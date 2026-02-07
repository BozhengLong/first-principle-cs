#include "types.h"
#include "defs.h"

// String manipulation functions

void* memset(void *dst, int c, uint n) {
    char *d = (char*)dst;
    while (n-- > 0) {
        *d++ = c;
    }
    return dst;
}

void* memcpy(void *dst, const void *src, uint n) {
    char *d = (char*)dst;
    const char *s = (const char*)src;
    while (n-- > 0) {
        *d++ = *s++;
    }
    return dst;
}

void* memmove(void *dst, const void *src, uint n) {
    const char *s = (const char*)src;
    char *d = (char*)dst;

    if (s < d && s + n > d) {
        // Overlap: copy backwards
        s += n;
        d += n;
        while (n-- > 0) {
            *--d = *--s;
        }
    } else {
        // No overlap or safe: copy forwards
        while (n-- > 0) {
            *d++ = *s++;
        }
    }
    return dst;
}

int memcmp(const void *v1, const void *v2, uint n) {
    const uint8_t *s1 = (const uint8_t*)v1;
    const uint8_t *s2 = (const uint8_t*)v2;

    while (n-- > 0) {
        if (*s1 != *s2) {
            return *s1 - *s2;
        }
        s1++;
        s2++;
    }
    return 0;
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
    return (uint8_t)*p - (uint8_t)*q;
}

int strncmp(const char *p, const char *q, uint n) {
    while (n > 0 && *p && *p == *q) {
        n--;
        p++;
        q++;
    }
    if (n == 0) {
        return 0;
    }
    return (uint8_t)*p - (uint8_t)*q;
}

char* strncpy(char *s, const char *t, int n) {
    char *os = s;
    while (n-- > 0 && (*s++ = *t++) != 0)
        ;
    while (n-- > 0) {
        *s++ = 0;
    }
    return os;
}
