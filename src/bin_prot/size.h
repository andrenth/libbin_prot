#ifndef BIN_PROT_SIZE_H
#define BIN_PROT_SIZE_H

#include <sys/types.h>

typedef size_t (*bin_sizer)(void *v);

size_t bin_size_unit(void *_unused);
size_t bin_size_bool(int *_unused);
size_t bin_size_char(char *_unused);

size_t bin_size_nat0(size_t *np);

size_t bin_size_int(long *np);
size_t bin_size_int32(int32_t *np);
size_t bin_size_int64(int64_t *np);

size_t bin_size_int_8bit(long *_unused);
size_t bin_size_int_16bit(long *_unused);
size_t bin_size_int_32bit(long *_unused);
size_t bin_size_int_64bit(long *_unused);

size_t bin_size_network16_int(int16_t *_unused);
size_t bin_size_network32_int(int32_t *_unused);
size_t bin_size_network64_int(int64_t *_unused);

size_t bin_size_variant_int(int32_t *_unused);

size_t bin_size_float(double *_unused);

size_t bin_size_string(const char *s);

size_t bin_size_option(bin_sizer bin_size_el, void *opt);

size_t bin_size_pair(bin_sizer bin_size_a, bin_sizer bin_size_b,
                     void *a, void *b);

size_t bin_size_triple(bin_sizer bin_size_a,
                       bin_sizer bin_size_b,
                       bin_sizer bin_size_c,
                       void *a, void *b, void *c);

size_t bin_size_array(bin_sizer bin_size_el, void **array, size_t len);

size_t bin_size_digest(const char *_unused);

#endif
