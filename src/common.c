#include <sys/types.h>
#include <stdint.h>

#include "common.h"

uint32_t
unsafe_get32(void *buf, size_t pos)
{
    unsigned char b1 = ((unsigned char *)buf)[pos];
    unsigned char b2 = ((unsigned char *)buf)[pos + 1];
    unsigned char b3 = ((unsigned char *)buf)[pos + 2];
    unsigned char b4 = ((unsigned char *)buf)[pos + 3];
    if (arch_big_endian())
        return b1 << 24 | b2 << 16 | b3 << 8 | b4;
    else
        return b4 << 24 | b3 << 16 | b2 << 8 | b1;
}

uint64_t
unsafe_get64(void *buf, size_t pos)
{
    unsigned char b1 = ((unsigned char *)buf)[pos];
    unsigned char b2 = ((unsigned char *)buf)[pos + 1];
    unsigned char b3 = ((unsigned char *)buf)[pos + 2];
    unsigned char b4 = ((unsigned char *)buf)[pos + 3];
    unsigned char b5 = ((unsigned char *)buf)[pos + 4];
    unsigned char b6 = ((unsigned char *)buf)[pos + 5];
    unsigned char b7 = ((unsigned char *)buf)[pos + 6];
    unsigned char b8 = ((unsigned char *)buf)[pos + 7];
    if (arch_big_endian())
        return (uint64_t)b1 << 56 | (uint64_t)b2 << 48 | (uint64_t)b3 << 40
             | (uint64_t)b4 << 32 | (uint64_t)b5 << 24 | (uint64_t)b6 << 16
             | (uint64_t)b7 << 8  | (uint64_t)b8;
    else
        return (uint64_t)b8 << 56 | (uint64_t)b7 << 48 | (uint64_t)b6 << 40
             | (uint64_t)b5 << 32 | (uint64_t)b4 << 24 | (uint64_t)b3 << 16
             | (uint64_t)b2 << 8  | (uint64_t)b1;
}

void
unsafe_set32(void *buf, size_t i, int32_t n)
{
    unsigned char b1, b2, b3, b4;

    if (arch_big_endian()) {
        b1 = 0xff & n >> 24;
        b2 = 0xff & n >> 16;
        b3 = 0xff & n >> 8;
        b4 = 0xff & n;
    } else {
        b4 = 0xff & n >> 24;
        b3 = 0xff & n >> 16;
        b2 = 0xff & n >> 8;
        b1 = 0xff & n;
    }
    ((unsigned char*)buf)[i + 0] = b1;
    ((unsigned char*)buf)[i + 1] = b2;
    ((unsigned char*)buf)[i + 2] = b3;
    ((unsigned char*)buf)[i + 3] = b4;
}

void
unsafe_set64(void *buf, size_t i, int64_t n)
{
    unsigned char b1, b2, b3, b4, b5, b6, b7, b8;

    if (arch_big_endian()) {
        b1 = 0xff & n >> 56;
        b2 = 0xff & n >> 48;
        b3 = 0xff & n >> 40;
        b4 = 0xff & n >> 32;
        b5 = 0xff & n >> 24;
        b6 = 0xff & n >> 16;
        b7 = 0xff & n >> 8;
        b8 = 0xff & n;
    } else {
        b8 = 0xff & n >> 56;
        b7 = 0xff & n >> 48;
        b6 = 0xff & n >> 40;
        b5 = 0xff & n >> 32;
        b4 = 0xff & n >> 24;
        b3 = 0xff & n >> 16;
        b2 = 0xff & n >> 8;
        b1 = 0xff & n;
    }
    ((unsigned char*)buf)[i + 0] = b1;
    ((unsigned char*)buf)[i + 1] = b2;
    ((unsigned char*)buf)[i + 2] = b3;
    ((unsigned char*)buf)[i + 3] = b4;
    ((unsigned char*)buf)[i + 4] = b5;
    ((unsigned char*)buf)[i + 5] = b6;
    ((unsigned char*)buf)[i + 6] = b7;
    ((unsigned char*)buf)[i + 7] = b8;
}

int64_t
unsafe_get64le(void *buf, size_t pos)
{
    int64_t x = unsafe_get64(buf, pos);
    if (arch_big_endian())
        return bswap64(x);
    return x;
}
