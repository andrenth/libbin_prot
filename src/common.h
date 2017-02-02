#ifndef BIN_PROT_COMMON_H
#define BIN_PROT_COMMON_H

#include <byteswap.h>

#define CODE_NEG_INT8 '\xff'
#define CODE_INT16 '\xfe'
#define CODE_INT32 '\xfd'
#define CODE_INT64 '\xfc'

#define OCAML_INT_MAX_32 (INT32_MAX >> 1)
#define OCAML_INT_MAX_64 (INT64_MAX >> 1)
#define OCAML_INT_MAX    (LONG_MAX  >> 1)

#define OCAML_INT_MIN_32 (-OCAML_INT_MAX_32 - 1)
#define OCAML_INT_MIN_64 (-OCAML_INT_MAX_64 - 1)
#define OCAML_INT_MIN    (-OCAML_INT_MAX - 1)

#define bswap16(x) bswap_16(x)
#define bswap32(x) bswap_32(x)
#define bswap64(x) bswap_64(x)

static inline int
arch_sixtyfour(void)
{
    return (8 * sizeof(void*) == 64);
}

static inline int
arch_big_endian(void)
{
    int i = 1;
    return !*((char *)&i);
}

uint32_t unsafe_get32(void *buf, size_t pos);
uint64_t unsafe_get64(void *buf, size_t pos);
void unsafe_set32(void *buf, size_t i, int32_t n);
void unsafe_set64(void *buf, size_t i, int64_t n);

#endif
