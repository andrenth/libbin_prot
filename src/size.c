#include <sys/types.h>

#include <stdint.h>
#include <string.h>

#include "common.h"

typedef size_t (*sizer)(void *v);

size_t
bin_size_unit(void)
{
    return 1;
}

size_t
bin_size_bool(int _unused)
{
    return 1;
}

size_t
bin_size_nat0(size_t n)
{
    if (n < 0x00000080)
        return 1;
    else if (n < 0x00010000)
        return 3;
    else if (arch_sixtyfour() && n >= 0x100000000)
        return 9;
    else
        return 5;
}

size_t
bin_size_string(char *s)
{
    size_t len = strlen(s);
    return bin_size_nat0(len) + len;
}

size_t
bin_size_char(char _unused)
{
    return 1;
}

static size_t
bin_size_int_nat0(size_t n)
{
    if (n < 0x00000080)
        return 1;
    else if (n < 0x00008000)
        return 3;
    else if (arch_sixtyfour() && n >= 0x80000000)
        return 9;
    else
        return 5;
}

static size_t
bin_size_int_negative(size_t n)
{
    if (n >= -0x00000080)
        return 2;
    else if (n >= -0x00008000)
        return 3;
    else if (n < -0x80000000)
        return 9;
    else
        return 5;
}

size_t
bin_size_int(long n)
{
    return (n >= 0) ? bin_size_int_nat0(n) : bin_size_int_negative(n);
}

size_t
bin_size_float(double _unused)
{
    return 8;
}

size_t
bin_size_int32(int32_t n)
{
    if (arch_sixtyfour())
        return bin_size_int(n);
    else if (n >= 0x00008000 || n < -0x00008000)
        return 5;
    else
        return bin_size_int(n);
}

size_t
bin_size_int64(int64_t n)
{
    if (arch_sixtyfour())
        return (n >= 0x80000000 || n < -0x80000000) ? 9 : bin_size_int(n);
    else
        return (n >= 0x80000000 || n < -0x80000000) ? 5 : bin_size_int32(n);
}

size_t
bin_size_option(sizer bin_size_el, void *opt)
{
    if (opt == NULL)
        return 1;
    return 1 + bin_size_el(opt);
}

size_t
bin_size_pair(sizer bin_size_a, sizer bin_size_b, void *a, void *b)
{
    return bin_size_a(a) + bin_size_b(b);
}

size_t
bin_size_triple(sizer bin_size_a, sizer bin_size_b, sizer bin_size_c,
                void *a, void *b, void *c)
{
    return bin_size_a(a) + bin_size_b(b) + bin_size_c(c);
}

size_t
bin_size_array(sizer bin_size_el, void **array, size_t len)
{
    size_t i, size;
    for (i = 0, size = 0; i < len; i++)
        size += bin_size_el(array[i]);
    return size;
}

size_t
bin_size_variant_int(int32_t _unused)
{
    return 4;
}

size_t
bin_size_int_8bit(int8_t _unused)
{
    return 1;
}

size_t
bin_size_int_16bit(int16_t _unused)
{
    return 2;
}

size_t
bin_size_int_32bit(int32_t _unused)
{
    return 4;
}

size_t
bin_size_int_64bit(int64_t _unused)
{
    return 8;
}

size_t
bin_size_network16_int(int16_t _unused)
{
    return 2;
}

size_t
bin_size_network32_int(int32_t _unused)
{
    return 4;
}

size_t
bin_size_network64_int(int64_t _unused)
{
    return 8;
}
