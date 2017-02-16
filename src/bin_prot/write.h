#ifndef BIN_PROT_WRITE_H
#define BIN_PROT_WRITE_H

#include <sys/types.h>

typedef size_t (*bin_writer)(void *buf, size_t pos, void *v);

size_t bin_write_unit(void *buf, size_t pos, void *_unused);
size_t bin_write_bool(void *buf, size_t pos, int v);
size_t bin_write_char(void *buf, size_t pos, char c);

size_t bin_write_nat0(void *buf, size_t pos, unsigned long n);
size_t bin_write_int(void *buf, size_t pos, long n);
size_t bin_write_int32(void *buf, size_t pos, int32_t n);
size_t bin_write_int64(void *buf, size_t pos, int64_t n);

size_t bin_write_int_8bit(void *buf, size_t pos, long n);
size_t bin_write_int_16bit(void *buf, size_t pos, long n);
size_t bin_write_int_32bit(void *buf, size_t pos, long n);
size_t bin_write_int_64bit(void *buf, size_t pos, long n);

size_t bin_write_int64_bits(void *buf, size_t pos, int64_t n);

size_t bin_write_network16_int(void *buf, size_t pos, long n);
size_t bin_write_network32_int(void *buf, size_t pos, long n);
size_t bin_write_network64_int(void *buf, size_t pos, long n);

size_t bin_write_network16_int16(void *buf, size_t pos, int16_t n);
size_t bin_write_network32_int32(void *buf, size_t pos, int32_t n);
size_t bin_write_network64_int64(void *buf, size_t pos, int64_t n);

size_t bin_write_variant_int(void *buf, size_t pos, int32_t i);

size_t bin_write_float(void *buf, size_t pos, double x);

size_t bin_write_string(void *buf, size_t pos, const char *s);

size_t bin_write_option(bin_writer bin_write_el,
                        void *buf, size_t pos, void *opt);

size_t bin_write_pair(bin_writer bin_write_a, bin_writer bin_write_b,
                      void *buf, size_t pos, void *a, void *b);

size_t bin_write_triple(bin_writer bin_write_a,
                        bin_writer bin_write_b,
                        bin_writer bin_write_c,
                        void *buf, size_t pos, void *a, void *b, void *c);

size_t bin_write_array(bin_writer bin_write_el,
                       void *buf, size_t pos, void **array, size_t len);

size_t bin_write_digest(void *buf, size_t pos, char *s);

#endif
