#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <bin_prot/read.h>

#include "common.h"

#include <stdio.h>

static int8_t
unsafe_get8_signed(void *buf, size_t pos)
{
    int8_t c = ((int8_t *)buf)[pos];
    if (c >= 128)
        return c - 256;
    return c;
}

static inline int32_t
max_int_int32()
{
    if (arch_sixtyfour())
        return INT32_MAX;
    return (int32_t)OCAML_INT_MAX;
}

static inline int32_t
min_int_int32()
{
    if (arch_sixtyfour())
        return INT32_MIN;
    return (int32_t)OCAML_INT_MIN;
}

static inline int64_t
max_int_int64()
{
    return OCAML_INT_MAX_64;
}

static inline int64_t
min_int_int64()
{
    return OCAML_INT_MIN_64;
}

static int
ocaml_int32_out_of_bounds(int32_t x)
{
    return
        !(arch_sixtyfour() || (x >= min_int_int32() && x <= max_int_int32()));
}

static int
ocaml_int64_out_of_bounds(int64_t x)
{
    return (x < min_int_int64() || x > max_int_int64());
}

static uint16_t
unsafe_get16(void *buf, size_t pos)
{
    unsigned char b1 = ((unsigned char *)buf)[pos];
    unsigned char b2 = ((unsigned char *)buf)[pos + 1];
    if (arch_big_endian())
        return b1 << 8 | b2;
    else
        return b2 << 8 | b1;
}

static int16_t
unsafe_get16be_unsigned(void *buf, size_t pos)
{
    int16_t x = unsafe_get16(buf, pos);
    if (arch_big_endian())
        return x;
    return bswap16(x);
}

static int32_t
unsafe_get32be(void *buf, size_t pos)
{
    int32_t x = unsafe_get32(buf, pos);
    if (arch_big_endian())
        return x;
    return bswap32(x);
}

static int64_t
unsafe_get64be(void *buf, size_t pos)
{
    int64_t x = unsafe_get64(buf, pos);
    if (arch_big_endian())
        return x;
    return bswap64(x);
}

static int16_t
unsafe_get16le_unsigned(void *buf, size_t pos)
{
    int16_t x = unsafe_get16(buf, pos);
    if (arch_big_endian())
        return bswap16(x);
    return x;
}

static int32_t
unsafe_get32le(void *buf, size_t pos)
{
    int32_t x = unsafe_get32(buf, pos);
    if (arch_big_endian())
        return bswap32(x);
    return x;
}

static int64_t
unsafe_get16le_signed(void *buf, size_t pos)
{
    int16_t x = unsafe_get16le_unsigned(buf, pos);
    if (x > 32767)
        return x - 65536;
    return x;
}

int
bin_read_unit(void *buf, size_t *pos)
{
    unsigned char c = ((unsigned char *)buf)[*pos];
    if (c != '\000')
        return -1;
    *pos += 1;
    return 0;
}

int
bin_read_bool(void *buf, size_t *pos, int *res)
{
    switch (((char *)buf)[*pos]) {
    case '\000':
        *pos += 1;
        *res = 0;
        return 0;
    case '\001':
        *pos += 1;
        *res = 1;
        return 0;
    default:
        return -1;
    }
}

static int
safe_bin_read_neg_int8(void *buf, size_t *pos_ref, size_t pos, int8_t *res)
{
    int8_t n = unsafe_get8_signed(buf, pos);
    if (n >= 0)
        return -1;
    *pos_ref = pos + 1;
    *res = n;
    return 0;
}

static int
safe_bin_read_int16(void *buf, size_t *pos_ref, size_t pos, int16_t *res)
{
    *pos_ref = pos + 2;
    *res = unsafe_get16le_signed(buf, pos);
    return 0;
}

static int
safe_bin_read_int32(void *buf, size_t *pos_ref, size_t pos, int32_t *res)
{
    *pos_ref = pos + 4;
    *res = unsafe_get32le(buf, pos);
    return 0;
}

static int
safe_bin_read_int64(void *buf, size_t *pos_ref, size_t pos, int64_t *res)
{
    *pos_ref = pos + 8;
    *res = unsafe_get64le(buf, pos);
    return 0;
}

static int
safe_bin_read_int32_as_int(void *buf, size_t *pos_ref, size_t pos, long *res)
{
    int32_t x = unsafe_get32le(buf, pos);
    if (ocaml_int32_out_of_bounds(x))
        return -1;
    *pos_ref = pos + 4;
    *res = x;
    return 0;
}

static int
safe_bin_read_int64_as_int(void *buf, size_t *pos_ref, size_t pos, long *res)
{
    int64_t x = unsafe_get64le(buf, pos);
    if (ocaml_int64_out_of_bounds(x))
        return -1;
    *pos_ref = pos + 8;
    *res = x;
    return 0;
}

static int
safe_bin_read_int32_as_int64(void *buf, size_t *pos_ref, size_t pos,
                             int64_t *res)
{
    *pos_ref = pos + 4;
    *res = (int64_t)unsafe_get32le(buf, pos);
    return 0;
}

int
bin_read_nat0(void *buf, size_t *pos, unsigned long *res)
{
    char c = ((char *)buf)[*pos];
    if (c >= '\x00' && c <= '\x7f') {
        *pos += 1;
        *res = c;
        return 0;
    }
    switch (c) {
    case CODE_INT16:
        return safe_bin_read_int16(buf, pos, *pos + 1, (int16_t *)res);
    case CODE_INT32:
        return safe_bin_read_int32(buf, pos, *pos + 1, (int32_t *)res);
    case CODE_INT64:
        return safe_bin_read_int64(buf, pos, *pos + 1, (int64_t *)res);
    default:
        return -1;
    }
}

int
bin_read_string(void *buf, size_t *pos, char **res)
{
    size_t len;
    int ret;

    ret = bin_read_nat0(buf, pos, &len);
    if (ret < 0)
        return -1;

    size_t next = *pos + len;
    char *s = malloc(len + 1);

    memcpy(s, buf + *pos, len);
    s[len] = '\0';
    *pos = next;
    *res = s;

    return 0;
}

int
bin_read_char(void *buf, size_t *pos, unsigned char *res)
{
    *res = ((unsigned char *)buf)[*pos];
    *pos += 1;
    return 0;
}

int
bin_read_int32(void *buf, size_t *pos, int32_t *res)
{
    char c = ((char *)buf)[*pos];
    if (c >= '\x00' && c <= '\x7f') {
        *pos += 1;
        *res = (int32_t)c & 0xff;
        return 0;
    }
    switch (c) {
    case CODE_NEG_INT8:
        return safe_bin_read_neg_int8(buf, pos, *pos + 1, (int8_t *)res);
    case CODE_INT16:
        return safe_bin_read_int16(buf, pos, *pos + 1, (int16_t *)res);
    case CODE_INT32:
        return safe_bin_read_int32(buf, pos, *pos + 1, res);
    default:
        return -1;
    }
}

int
bin_read_int64(void *buf, size_t *pos, int64_t *res)
{
    char c = ((char *)buf)[*pos];
    if (c >= '\x00' && c <= '\x7f') {
        *pos += 1;
        *res = (int64_t)c & 0xff;
        return 0;
    }
    switch (c) {
    case CODE_NEG_INT8:
        return safe_bin_read_neg_int8(buf, pos, *pos + 1, (int8_t *)res);
    case CODE_INT16:
        return safe_bin_read_int16(buf, pos, *pos + 1, (int16_t *)res);
    case CODE_INT32:
        return safe_bin_read_int32_as_int64(buf, pos, *pos + 1, res);
    case CODE_INT64:
        return safe_bin_read_int64(buf, pos, *pos + 1, res);
    default:
        return -1;
    }
}

int
bin_read_int(void *buf, size_t *pos, long *res)
{
    char c = ((char *)buf)[*pos];
    if (c >= '\x00' && c <= '\x7f') {
        *pos += 1;
        *res = (long)c & 0xff;
        return 0;
    }
    switch (c) {
    case CODE_NEG_INT8:
        return safe_bin_read_neg_int8(buf, pos, *pos + 1, (int8_t *)res);
    case CODE_INT16:
        return safe_bin_read_int16(buf, pos, *pos + 1, (int16_t *)res);
    case CODE_INT32:
        return safe_bin_read_int32_as_int(buf, pos, *pos + 1, res);
    case CODE_INT64:
        return safe_bin_read_int64_as_int(buf, pos, *pos + 1, res);
    default:
        return -1;
    }
}

int
bin_read_variant_int(void *buf, size_t *pos, long *res)
{
    int32_t n = unsafe_get32le(buf, *pos);
    if ((n & 1) == 0)
        return -1;
    *pos += 4;
    *res = (long)(n >> 1);
    return 0;
}

int
bin_read_int_8bit(void *buf, size_t *pos, long *res)
{
    *res = ((int8_t *)buf)[*pos];
    *pos += 1;
    return 0;
}

int
bin_read_int_16bit(void *buf, size_t *pos, long *res)
{
    *res = (uint16_t)unsafe_get16le_unsigned(buf, *pos);
    *pos += 2;
    return 0;
}

int
bin_read_int_32bit(void *buf, size_t *pos, long *res)
{
    int32_t n = unsafe_get32le(buf, *pos);
    if (ocaml_int32_out_of_bounds(n))
        return -1;
    *pos += 4;
    *res = n;
    return 0;
}

int
bin_read_int_64bit(void *buf, size_t *pos, long *res)
{
    int64_t n = unsafe_get64le(buf, *pos);
    if (ocaml_int64_out_of_bounds(n))
        return -1;
    *pos += 8;
    *res = n;
    return 0;
}

int
bin_read_int64_bits(void *buf, size_t *pos, int64_t *res)
{
    *res = unsafe_get64le(buf, *pos);
    *pos += 8;
    return 0;
}

int
bin_read_network16_int16(void *buf, size_t *pos, int16_t *res)
{
    *res = unsafe_get16be_unsigned(buf, *pos);
    *pos += 2;
    return 0;
}

int
bin_read_network16_int(void *buf, size_t *pos, long *res)
{
    return bin_read_network16_int16(buf, pos, (int16_t *)res);
}

int
bin_read_network32_int(void *buf, size_t *pos, long *res)
{
    int32_t n = unsafe_get32be(buf, *pos);
    if (ocaml_int32_out_of_bounds(n))
        return -1;
    *pos += 4;
    *res = n;
    return 0;
}

int
bin_read_network32_int32(void *buf, size_t *pos, int32_t *res)
{
    *res = unsafe_get32be(buf, *pos);
    *pos += 4;
    return 0;
}

int
bin_read_network64_int(void *buf, size_t *pos, long *res)
{
    int64_t n = unsafe_get64be(buf, *pos);
    if (ocaml_int64_out_of_bounds(n))
        return -1;
    *pos += 8;
    *res = n;
    return 0;
}

int
bin_read_network64_int64(void *buf, size_t *pos, int64_t *res)
{
    *res = unsafe_get64be(buf, *pos);
    *pos += 8;
    return 0;
}

int
bin_read_option(bin_reader bin_read_el, void *buf, size_t *pos, void *res)
{
    char c = ((char *)buf)[*pos];
    switch(c) {
    case '\000':
        *pos += 1;
        return 0;
    case '\001':
        *pos += 1;
        if (bin_read_el(buf, pos, res) < 0)
            return -1;
        return 1;
    default:
        return -1;
    }
}

int
bin_read_pair(bin_reader bin_read_a, bin_reader bin_read_b,
              void *buf, size_t *pos, void *res1, void *res2)
{
    if (bin_read_a(buf, pos, res1) < 0)
        return -1;
    if (bin_read_b(buf, pos, res2) < 0)
        return -1;
    return 0;
}

int
bin_read_triple(bin_reader bin_read_a,
                bin_reader bin_read_b,
                bin_reader bin_read_c,
                void *buf, size_t *pos, void *res1, void *res2, void *res3)
{
    if (bin_read_a(buf, pos, res1) < 0)
        return -1;
    if (bin_read_b(buf, pos, res2) < 0)
        return -1;
    if (bin_read_c(buf, pos, res3) < 0)
        return -1;
    return 0;
}

static double
float_of_bits(int64_t i)
{
    union { double d; int64_t i; int32_t h[2]; } u;
    u.i = i;
    return u.d;
}

int
bin_read_float(void *buf, size_t *pos, double *res)
{
    int64_t i = unsafe_get64le(buf, *pos);
    *pos += 8;
    *res = float_of_bits(i);
    return 0;
}

#define BIN_READ_ARRAY(name, type)                                       \
int                                                                      \
bin_read_##name##_array(void *buf, size_t *pos, size_t *len, type **res) \
{                                                                        \
    unsigned long i;                                                     \
    if (bin_read_nat0(buf, pos, len) < 0)                                \
        return -1;                                                       \
    type *array = malloc(*len * sizeof(type));                           \
    for (i = 0; i < *len; i++) {                                         \
        type r;                                                          \
        if (bin_read_##name(buf, pos, &r) < 0)                           \
            return -1;                                                   \
        array[i] = r;                                                    \
    }                                                                    \
    *res = array;                                                        \
    return 0;                                                            \
}

BIN_READ_ARRAY(bool,            int);
BIN_READ_ARRAY(char,            unsigned char);
BIN_READ_ARRAY(int,             long);
BIN_READ_ARRAY(int32,           int32_t);
BIN_READ_ARRAY(int64,           int64_t);
BIN_READ_ARRAY(int_8bit,        long);
BIN_READ_ARRAY(int_16bit,       long);
BIN_READ_ARRAY(int_32bit,       long);
BIN_READ_ARRAY(int_64bit,       long);
BIN_READ_ARRAY(int64_bits,      int64_t);
BIN_READ_ARRAY(network16_int,   long);
BIN_READ_ARRAY(network32_int,   long);
BIN_READ_ARRAY(network64_int,   long);
BIN_READ_ARRAY(network16_int16, int16_t);
BIN_READ_ARRAY(network32_int32, int32_t);
BIN_READ_ARRAY(network64_int64, int64_t);
BIN_READ_ARRAY(variant_int,     long);
BIN_READ_ARRAY(float,           double);

int
bin_read_string_array(void *buf, size_t *pos, size_t *len, char ***res)
{
    unsigned long i;
    if (bin_read_nat0(buf, pos, len) < 0)
        return -1;
    char **array = malloc(*len);
    for (i = 0; i < *len; i++) {
        char *s;
        if (bin_read_string(buf, pos, &s) < 0)
            return -1;
        array[i] = strdup(s);
    }
    *res = array;
    return 0;
}

int
bin_read_digest(void *buf, size_t *pos, char **res)
{
    size_t p = *pos;
    *pos += 16;
    char *b = malloc(16);
    if (arch_sixtyfour()) {
        uint64_t a = unsafe_get64(buf, p);
        uint64_t b = unsafe_get64(buf, p + 8);
        unsafe_set64(buf, 0, a);
        unsafe_set64(buf, 8, b);
    } else {
        uint64_t a = unsafe_get64(buf, p);
        uint64_t b = unsafe_get64(buf, p +  4);
        uint64_t c = unsafe_get64(buf, p +  8);
        uint64_t d = unsafe_get64(buf, p + 16);
        unsafe_set64(buf,  0, a);
        unsafe_set64(buf,  4, b);
        unsafe_set64(buf,  8, c);
        unsafe_set64(buf, 12, d);
    }
    *res = b;
    return 0;
}
