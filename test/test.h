#ifndef TEST_H
#define TEST_H

#include <sys/types.h>

#define TEST_EXPECTATIONS_FILE "test/integers_repr.expected"

char *test_parse_line(char *line, int64_t *res);
char *test_bin_file(char *type);

#endif
