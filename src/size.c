#include <sys/types.h>

#include <stdint.h>
#include <string.h>

#include <bin_prot/size.h>

#include "common.h"

size_t
bin_size_unit(void *_unused)
{
    return 1;
}

size_t
bin_size_bool(int *_unused)
{
    return 1;
}

size_t
bin_size_nat0(size_t *np)
{
    size_t n = *np;

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
bin_size_string(const char *s)
{
    size_t len = strlen(s);
    size_t x = bin_size_nat0(&len) + len;
    return x;
}

size_t
bin_size_char(unsigned char *_unused)
{
    return 1;
}

static size_t
bin_size_int_nat0(unsigned long *np)
{
    size_t n = *np;

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
bin_size_int_negative(long *np)
{
    size_t n = *np;

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
bin_size_int(long *np)
{
    long n = *np;
    return (n >= 0) ? bin_size_int_nat0((unsigned long *)np)
                    : bin_size_int_negative(np);
}

size_t
bin_size_float(double *_unused)
{
    return 8;
}

size_t
bin_size_int32(int32_t *np)
{
    int32_t n = *np;

    if (arch_sixtyfour())
        return bin_size_int((long *)np);
    else if (n >= 0x00008000 || n < -0x00008000)
        return 5;
    else
        return bin_size_int((long *)np);
}

size_t
bin_size_int64(int64_t *np)
{
    int64_t n = *np;

    if (arch_sixtyfour())
        return (n >= 0x80000000 || n < -0x80000000) ? 9 : bin_size_int(np);
    else
        return (n >= 0x80000000 || n < -0x80000000)
             ? 5
             : bin_size_int32((int32_t *)np);
}

size_t
bin_size_option(bin_sizer bin_size_el, void *opt)
{
    if (opt == NULL)
        return 1;
    return 1 + bin_size_el(opt);
}

size_t
bin_size_pair(bin_sizer bin_size_a, bin_sizer bin_size_b, void *a, void *b)
{
    return bin_size_a(a) + bin_size_b(b);
}

size_t
bin_size_triple(bin_sizer bin_size_a,
                bin_sizer bin_size_b,
                bin_sizer bin_size_c,
                void *a, void *b, void *c)
{
    return bin_size_a(a) + bin_size_b(b) + bin_size_c(c);
}

size_t
bin_size_array(bin_sizer bin_size_el, void **array, size_t len)
{
    size_t i;
    size_t size = bin_size_nat0(&len);

    for (i = 0; i < len; i++)
        size += bin_size_el(array[i]);
    return size;
}

size_t
bin_size_list(bin_sizer bin_size_el, void **list, size_t len)
{
    return bin_size_array(bin_size_el, list, len);
}

size_t
bin_size_variant_int(long *_unused)
{
    return 4;
}

size_t
bin_size_int_8bit(long *_unused)
{
    return 1;
}

size_t
bin_size_int_16bit(long *_unused)
{
    return 2;
}

size_t
bin_size_int_32bit(long *_unused)
{
    return 4;
}

size_t
bin_size_int_64bit(long *_unused)
{
    return 8;
}

size_t
bin_size_network16_int(long *_unused)
{
    return 2;
}

size_t
bin_size_network32_int(long *_unused)
{
    return 4;
}

size_t
bin_size_network64_int(long *_unused)
{
    return 8;
}

size_t
bin_size_network16_int16(int16_t *_unused)
{
    return 2;
}

size_t
bin_size_network32_int32(int32_t *_unused)
{
    return 4;
}

size_t
bin_size_network64_int64(int64_t *_unused)
{
    return 8;
}

size_t
bin_size_digest(const char *_unused)
{
    return 16;
}
