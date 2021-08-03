/*
 * Copyright 2014, General Dynamics C4 Systems
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include <assert.h>
#include <stdint.h>
#include <util.h>

/*
 * memzero needs a custom type that allows us to use a word
 * that has the aliasing properties of a char.
 */
typedef unsigned long __attribute__((__may_alias__)) ulong_alias;

/*
 * Zero 'n' bytes of memory starting from 's'.
 *
 * 'n' and 's' must be word aligned.
 */
void memzero(void *s, unsigned long n)
{
    uint8_t *p = s;

    /* Ensure alignment constraints are met. */
    assert((unsigned long)s % sizeof(unsigned long) == 0);
    assert(n % sizeof(unsigned long) == 0);

    /* We will never memzero an area larger than the largest current
       live object */
    /** GHOSTUPD: "(gs_get_assn cap_get_capSizeBits_'proc \<acute>ghost'state = 0
        \<or> \<acute>n <= gs_get_assn cap_get_capSizeBits_'proc \<acute>ghost'state, id)" */

    /* Write out words. */
    while (n != 0) {
        *(ulong_alias *)p = 0;
        p += sizeof(ulong_alias);
        n -= sizeof(ulong_alias);
    }
}

long CONST char_to_long(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    return -1;
}

long PURE str_to_long(const char *str)
{
    unsigned int base;
    long res;
    long val = 0;
    char c;

    /*check for "0x" */
    if (*str == '0' && (*(str + 1) == 'x' || *(str + 1) == 'X')) {
        base = 16;
        str += 2;
    } else {
        base = 10;
    }

    if (!*str) {
        return -1;
    }

    c = *str;
    while (c != '\0') {
        res = char_to_long(c);
        if (res == -1 || res >= base) {
            return -1;
        }
        val = val * base + res;
        str++;
        c = *str;
    }

    return val;
}

#ifdef CONFIG_ARCH_RISCV
// uint32_t __clzsi2(uint32_t x)
// {
//     uint32_t count = 0;
//     while (!(x & 0x80000000U) && count < 34) {
//         x <<= 1;
//         count++;
//     }
//     return count;
// }

// uint32_t __ctzsi2(uint32_t x)
// {
//     uint32_t count = 0;
//     while (!(x & 0x000000001) && count <= 32) {
//         x >>= 1;
//         count++;
//     }
//     return count;
// }

// uint32_t __clzdi2(uint64_t x)
// {
//     uint32_t count = 0;
//     while (!(x & 0x8000000000000000U) && count < 65) {
//         x <<= 1;
//         count++;
//     }
//     return count;
// }

// uint32_t __ctzdi2(uint64_t x)
// {
//     uint32_t count = 0;
//     while (!(x & 0x00000000000000001) && count <= 64) {
//         x >>= 1;
//         count++;
//     }
//     return count;
// }
#endif /* CONFIG_ARCH_RISCV */
