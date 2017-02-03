#ifndef TEST_H
#define TEST_H

#include <sys/types.h>

#define TEST_EXPECTATIONS_FILE "test/integers_repr.expected"

struct test {
    char *name;
};

struct line_data {
    char     type[32];
    int64_t  value;
    size_t   index;
    size_t   pos;
    int      will_be_tested;
    int      type_changed;
};

char *test_parse_line(char *line, int64_t *res);
char *test_bin_file(char *type);
int   test_read_line(FILE *fp, struct line_data *data, struct test *tests,
                     void *arg, void (*f)(char *bin_file, void *arg));

#endif
