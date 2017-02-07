#ifndef BIN_PROT_H
#define BIN_PROT_H

#include <sys/types.h>

typedef int    (*reader)(void *buf, size_t *pos, void *res);
typedef size_t (*writer)(void *buf, size_t pos, void *v);
typedef size_t (*sizer)(void *v);

/*
 * Readers
 */

int bin_read_unit(void *buf, size_t *pos);
int bin_read_bool(void *buf, size_t *pos, int *res);
int bin_read_char(void *buf, size_t *pos, unsigned char *res);

int bin_read_nat0(void *buf, size_t *pos, unsigned long *res);
int bin_read_int(void *buf, size_t *pos, long *res);
int bin_read_int32(void *buf, size_t *pos, int32_t *res);
int bin_read_int64(void *buf, size_t *pos, int64_t *res);

int bin_read_int_8bit(void *buf, size_t *pos, long *res);
int bin_read_int_16bit(void *buf, size_t *pos, long *res);
int bin_read_int_32bit(void *buf, size_t *pos, long *res);
int bin_read_int_64bit(void *buf, size_t *pos, long *res);

int bin_read_int64_bits(void *buf, size_t *pos, int64_t *res);

int bin_read_network16_int(void *buf, size_t *pos, long *res);
int bin_read_network32_int(void *buf, size_t *pos, long *res);
int bin_read_network64_int(void *buf, size_t *pos, long *res);

int bin_read_network16_int16(void *buf, size_t *pos, int16_t *res);
int bin_read_network32_int32(void *buf, size_t *pos, int32_t *res);
int bin_read_network64_int64(void *buf, size_t *pos, int64_t *res);

int bin_read_variant_int(void *buf, size_t *pos, long *res);

int bin_read_float(void *buf, size_t *pos, double *res);

int bin_read_string(void *buf, size_t *pos, char **res);

int bin_read_option(reader bin_read_el, void *buf, size_t *pos, void *res);

int bin_read_pair(reader bin_read_a, reader bin_read_b, void *buf, size_t *pos,
                  void *res1, void *res2);

int bin_read_triple(reader bin_read_a, reader bin_read_b, reader bin_read_c,
                    void *buf, size_t *pos, void *res1, void *res2, void *res3);

int bin_read_bool_array(void *buf, size_t *pos, size_t *len, int **res);
int bin_read_char_array(void *buf, size_t *pos, size_t *len, unsigned char **res);
int bin_read_int_array(void *buf, size_t *pos, size_t *len, long **res);
int bin_read_int32_array(void *buf, size_t *pos, size_t *len, int32_t **res);
int bin_read_int64_array(void *buf, size_t *pos, size_t *len, int64_t **res);
int bin_read_float_array(void *buf, size_t *pos, size_t *len, double **res);
int bin_read_string_array(void *buf, size_t *pos, size_t *len, char ***res);

int bin_read_digest(void *buf, size_t *pos, char **res);

/*
 * Writers
 */

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

size_t bin_write_string(void *buf, size_t pos, char *s);

size_t bin_write_option(writer bin_write_el, void *buf, size_t pos, void *opt);

size_t bin_write_pair(writer bin_write_a, writer bin_write_b,
                      void *buf, size_t pos, void *a, void *b);

size_t bin_write_triple(writer bin_write_a,
                        writer bin_write_b,
                        writer bin_write_c,
                        void *buf, size_t pos, void *a, void *b, void *c);

size_t bin_write_array(writer bin_write_el, void *buf, size_t pos, void **array,
                       size_t len);

size_t bin_write_digest(void *buf, size_t pos, char *s);

/*
 * Sizers
 */

size_t bin_size_unit(void *_unused);
size_t bin_size_bool(int _unused);
size_t bin_size_char(char _unused);

size_t bin_size_nat0(size_t n);

size_t bin_size_int(long n);
size_t bin_size_int32(int32_t n);
size_t bin_size_int64(int64_t n);

size_t bin_size_int_8bit(long _unused);
size_t bin_size_int_16bit(long _unused);
size_t bin_size_int_32bit(long _unused);
size_t bin_size_int_64bit(long _unused);

size_t bin_size_network16_int(int16_t _unused);
size_t bin_size_network32_int(int32_t _unused);
size_t bin_size_network64_int(int64_t _unused);

size_t bin_size_variant_int(int32_t _unused);

size_t bin_size_float(double _unused);

size_t bin_size_string(char *s);

size_t bin_size_option(sizer bin_size_el, void *opt);

size_t bin_size_pair(sizer bin_size_a, sizer bin_size_b, void *a, void *b);

size_t bin_size_triple(sizer bin_size_a, sizer bin_size_b, sizer bin_size_c,
                       void *a, void *b, void *c);

size_t bin_size_array(sizer bin_size_el, void **array, size_t len);

#endif
