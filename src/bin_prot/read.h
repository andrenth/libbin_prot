#ifndef BIN_PROT_READ_H
#define BIN_PROT_READ_H

#include <sys/types.h>

typedef int (*bin_reader)(void *buf, size_t *pos, void *res);

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

int bin_read_option(bin_reader bin_read_el, void *buf, size_t *pos, void *res);

int bin_read_pair(bin_reader bin_read_a, bin_reader bin_read_b,
                  void *buf, size_t *pos, void *res1, void *res2);

int bin_read_triple(bin_reader bin_read_a,
                    bin_reader bin_read_b,
                    bin_reader bin_read_c,
                    void *buf, size_t *pos,
                    void *res1, void *res2, void *res3);

int bin_read_bool_array(void *buf, size_t *pos, size_t *len, int **res);
int bin_read_char_array(void *buf, size_t *pos, size_t *len, unsigned char **res);
int bin_read_int_array(void *buf, size_t *pos, size_t *len, long **res);
int bin_read_int32_array(void *buf, size_t *pos, size_t *len, int32_t **res);
int bin_read_int64_array(void *buf, size_t *pos, size_t *len, int64_t **res);
int bin_read_int_8bit_array(void *buf, size_t *pos, size_t *len, long **res);
int bin_read_int_16bit_array(void *buf, size_t *pos, size_t *len, long **res);
int bin_read_int_32bit_array(void *buf, size_t *pos, size_t *len, long **res);
int bin_read_int_64bit_array(void *buf, size_t *pos, size_t *len, long **res);
int bin_read_int64_bits_array(void *buf, size_t *pos, size_t *len, int64_t **res);
int bin_read_network16_int_array(void *buf, size_t *pos, size_t *len, long **res);
int bin_read_network32_int_array(void *buf, size_t *pos, size_t *len, long **res);
int bin_read_network64_int_array(void *buf, size_t *pos, size_t *len, long **res);
int bin_read_network16_int16_array(void *buf, size_t *pos, size_t *len, int16_t **res);
int bin_read_network32_int32_array(void *buf, size_t *pos, size_t *len, int32_t **res);
int bin_read_network64_int64_array(void *buf, size_t *pos, size_t *len, int64_t **res);
int bin_read_variant_int_array(void *buf, size_t *pos, size_t *len, long **res);

int bin_read_float_array(void *buf, size_t *pos, size_t *len, double **res);
int bin_read_string_array(void *buf, size_t *pos, size_t *len, char ***res);

int bin_read_digest(void *buf, size_t *pos, char **res);

#endif
