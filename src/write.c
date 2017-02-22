#include <sys/types.h>

#include <stdint.h>
#include <string.h>

#include <bin_prot/write.h>

#include "common.h"

size_t
bin_write_unit(void *buf, size_t pos, void *_unused)
{
    char *b = (char *)buf;
    b[pos] = '\000';
    return pos + 1;
}

size_t
bin_write_bool(void *buf, size_t pos, int v)
{
    char *b = (char *)buf;
    b[pos] = v ? '\001' : '\000';
    return pos + 1;
}

static void
unsafe_set16(void *buf, size_t i, int16_t n)
{
    unsigned char b1, b2;

    if (arch_big_endian()) {
        b1 = 0xFF & n >> 8;
        b2 = 0xFF & n;
    } else {
        b2 = 0xFF & n >> 8;
        b1 = 0xFF & n;
    }
    ((unsigned char *)buf)[i + 0] = b1;
    ((unsigned char *)buf)[i + 1] = b2;
}

static void
unsafe_set16le(void *buf, size_t pos, int16_t n)
{
    if (arch_big_endian())
        unsafe_set16(buf, pos, bswap16(n));
    else
        unsafe_set16(buf, pos, n);
}

static void
unsafe_set16be(void *buf, size_t pos, int16_t n)
{
    if (arch_big_endian())
        unsafe_set16(buf, pos, n);
    else
        unsafe_set16(buf, pos, bswap16(n));
}

static void
unsafe_set32le(void *buf, size_t pos, int32_t n)
{
    if (arch_big_endian())
        unsafe_set32(buf, pos, bswap32(n));
    else
        unsafe_set32(buf, pos, n);
}

static void
unsafe_set32be(void *buf, size_t pos, int32_t n)
{
    if (arch_big_endian())
        unsafe_set32(buf, pos, n);
    else
        unsafe_set32(buf, pos, bswap32(n));
}

static void
unsafe_set64le(void *buf, size_t pos, int64_t n)
{
    if (arch_big_endian())
        unsafe_set64(buf, pos, bswap64(n));
    else
        unsafe_set64(buf, pos, n);
}

static void
unsafe_set64be(void *buf, size_t pos, int64_t n)
{
    if (arch_big_endian())
        unsafe_set64(buf, pos, n);
    else
        unsafe_set64(buf, pos, bswap64(n));
}

static size_t
all_bin_write_small_int(void *buf, size_t pos, int8_t n)
{
    ((int8_t *)buf)[pos] = n;
    return pos + 1;
}

static size_t
all_bin_write_neg_int8(void *buf, size_t pos, int8_t n)
{
    ((int8_t *)buf)[pos] = CODE_NEG_INT8;
    ((int8_t *)buf)[pos + 1] = n;
    return pos + 2;
}

static size_t
all_bin_write_int16(void *buf, size_t pos, int16_t n)
{
    ((int8_t *)buf)[pos] = CODE_INT16;
    unsafe_set16le(buf, pos + 1, n);
    return pos + 3;
}

static size_t
all_bin_write_int32(void *buf, size_t pos, int32_t n)
{
    ((int8_t *)buf)[pos] = CODE_INT32;
    unsafe_set32le(buf, pos + 1, n);
    return pos + 5;
}

static size_t
all_bin_write_int64(void *buf, size_t pos, int64_t n)
{
    ((int8_t *)buf)[pos] = CODE_INT64;
    unsafe_set64le(buf, pos + 1, n);
    return pos + 9;
}

size_t
bin_write_nat0(void *buf, size_t pos, unsigned long n)
{
    if (n < 0x00000080)
        return all_bin_write_small_int(buf, pos, n);
    else if (n < 0x00010000)
        return all_bin_write_int16(buf, pos, n);
    else if (arch_sixtyfour() && n >= (1l << 32))
        return all_bin_write_int64(buf, pos, n);
    else
        return all_bin_write_int32(buf, pos, n);
}

size_t
bin_write_int(void *buf, size_t pos, long n)
{
    if (n >= 0) {
        if (n < 0x00000080) {
            return all_bin_write_small_int(buf, pos, n);
        } else if (n < 0x00008000) {
            return all_bin_write_int16(buf, pos, n);
        } else if (arch_sixtyfour() && n >= (1l << 31)) {
            return all_bin_write_int64(buf, pos, n);
        } else {
            return all_bin_write_int32(buf, pos, n);
        }
    } else {
        if (n >= -0x00000080) {
            return all_bin_write_neg_int8(buf, pos, n);
        } else if (n >= -0x00008000) {
            return all_bin_write_int16(buf, pos, n);
        } else if (arch_sixtyfour() && n < -(1l << 31)) {
            return all_bin_write_int64(buf, pos, (int64_t)n);
        } else {
            return all_bin_write_int32(buf, pos, (int32_t)n);
        }
    }
}

size_t
bin_write_string(void *buf, size_t pos, const char *s)
{
    size_t len = strlen(s);
    size_t new_pos = bin_write_nat0(buf, pos, len);

    memcpy(buf + new_pos, s, len);

    return new_pos + len;
}

size_t
bin_write_char(void *buf, size_t pos, char c)
{
    ((char *)buf)[pos] = c;
    return pos + 1;
}

static int64_t
bits_of_float(double d)
{
    union { double d; int64_t i; int32_t h[2]; } u;
    u.d = d;
    return u.i;
}

size_t
bin_write_float(void *buf, size_t pos, double x)
{
    unsafe_set64le(buf, pos, bits_of_float(x));
    return pos + 8;
}

size_t
bin_write_int32(void *buf, size_t pos, int32_t n)
{
    if (arch_sixtyfour())
        return bin_write_int(buf, pos, n);

    if (n >= 0x00008000l || n < -0x00008000l) {
        return all_bin_write_int32(buf, pos, n);
    } else {
        return bin_write_int(buf, pos, n);
    }
}

size_t
bin_write_int64(void *buf, size_t pos, int64_t n)
{
    if (n >= 0x80000000L || n < -0x80000000L) {
        return all_bin_write_int64(buf, pos, n);
    } else if (arch_sixtyfour()) {
        return bin_write_int(buf, pos, n);
    } else if (n >= 0x00008000L || n < -0x00008000L) {
        return all_bin_write_int32(buf, pos, n);
    } else {
        return bin_write_int(buf, pos, n);
    }
}

size_t
bin_write_option(bin_writer bin_write_el, void *buf, size_t pos, void *opt)
{
    if (opt == NULL)
        return bin_write_bool(buf, pos, 0);
    size_t next = bin_write_bool(buf, pos, 1);
    return bin_write_el(buf, next, opt);
}

size_t
bin_write_pair(bin_writer bin_write_a, bin_writer bin_write_b,
               void *buf, size_t pos, void *a, void *b)
{
    pos = bin_write_a(buf, pos, a);
    return bin_write_b(buf, pos, b);
}

size_t
bin_write_triple(bin_writer bin_write_a,
                 bin_writer bin_write_b,
                 bin_writer bin_write_c,
                 void *buf, size_t pos, void *a, void *b, void *c)
{
    pos = bin_write_a(buf, pos, a);
    pos = bin_write_b(buf, pos, b);
    return bin_write_c(buf, pos, c);
}

size_t
bin_write_array(bin_writer bin_write_el, void *buf, size_t pos, void **array,
                size_t len)
{
    size_t i;

    pos = bin_write_nat0(buf, pos, len);
    for (i = 0; i < len; i++)
        pos = bin_write_el(buf, pos, array[i]);

    return pos;
}

size_t
bin_write_list(bin_writer bin_write_el, void *buf, size_t pos, void **list,
               size_t len)
{
    return bin_write_array(bin_write_el, buf, pos, list, len);
}

size_t
bin_write_digest(void *buf, size_t pos, char *s)
{
    //assert(strlen(s) == 16)
    if (arch_sixtyfour()) {
        uint64_t a = unsafe_get64(s, 0);
        uint64_t b = unsafe_get64(s, 8);
        unsafe_set64(buf, pos,     a);
        unsafe_set64(buf, pos + 8, b);
    } else {
        uint32_t a = unsafe_get32(s,  0);
        uint32_t b = unsafe_get32(s,  4);
        uint32_t c = unsafe_get32(s,  8);
        uint32_t d = unsafe_get32(s, 12);
        unsafe_set32(buf, pos,      a);
        unsafe_set32(buf, pos +  4, b);
        unsafe_set32(buf, pos +  8, c);
        unsafe_set32(buf, pos + 12, d);
    }
    return pos + 16;
}

size_t
bin_write_variant_int(void *buf, size_t pos, int32_t i)
{
    unsafe_set32le(buf, pos, (i << 1) | 1);
    return pos + 4;
}

size_t
bin_write_int_8bit(void *buf, size_t pos, long n)
{
    ((int8_t *)buf)[pos] = n;
    return pos + 1;
}

size_t
bin_write_int_16bit(void *buf, size_t pos, long n)
{
    unsafe_set16le(buf, pos, (int16_t)n);
    return pos + 2;
}

size_t
bin_write_int_32bit(void *buf, size_t pos, long n)
{
    unsafe_set32le(buf, pos, (int32_t)n);
    return pos + 4;
}

size_t
bin_write_int_64bit(void *buf, size_t pos, long n)
{
    unsafe_set64le(buf, pos, (int64_t)n);
    return pos + 8;
}

size_t
bin_write_int64_bits(void *buf, size_t pos, int64_t n)
{
    unsafe_set64le(buf, pos, n);
    return pos + 8;
}

size_t
bin_write_network16_int(void *buf, size_t pos, long n)
{
    unsafe_set16be(buf, pos, (int16_t)n);
    return pos + 2;
}

size_t
bin_write_network16_int16(void *buf, size_t pos, int16_t n)
{
    unsafe_set16be(buf, pos, n);
    return pos + 2;
}

size_t
bin_write_network32_int(void *buf, size_t pos, long n)
{
    unsafe_set32be(buf, pos, (int32_t)n);
    return pos + 4;
}

size_t
bin_write_network32_int32(void *buf, size_t pos, int32_t n)
{
    unsafe_set32be(buf, pos, n);
    return pos + 4;
}

size_t
bin_write_network64_int(void *buf, size_t pos, long n)
{
    unsafe_set64be(buf, pos, (int64_t)n);
    return pos + 8;
}

size_t
bin_write_network64_int64(void *buf, size_t pos, int64_t n)
{
    unsafe_set64be(buf, pos, n);
    return pos + 8;
}
