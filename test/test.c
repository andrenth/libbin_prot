#include <sys/types.h>
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct test {
    char *name;
    void *dummy;
};

struct line_data {
    char     type[32];
    int64_t  value;
    size_t   index;
    size_t   pos;
    int      will_be_tested;
    int      type_changed;
};

char *
test_parse_line(char *line, int64_t *res)
{
    char *p;
    char *r;
    long long i;

    r = strrchr(line, ' ');
    if (r == NULL)
        errx(1, "strrchr");
    r++;

    i = strtoll(r, NULL, 10);
    if (errno == ERANGE)
        err(1, "strtoll");
    *res = i;

    p = strchr(line, '|');
    if (p == NULL)
        errx(1, "strchr");
    *p = '\0';

    return line;
}

char *
test_bin_file(char *type)
{
    char *base = "_build/bin/";
    char *suff = ".bin";
    size_t siz = strlen(base) + strlen(suff) + strlen(type) + 1;
    char *buf  = malloc(siz);
    snprintf(buf, siz, "%s%s%s", base, type, suff);
    return buf;
}

static ssize_t
find_type(char *type, struct test *tests)
{
    ssize_t i;
    for (i = 0; tests[i].name != NULL; i++) {
        if (strcmp(type, tests[i].name) != 0)
            continue;
        return i;
    }
    return -1;
}


int
test_read_line(FILE *fp, struct line_data *data, struct test *tests,
               void *arg, void (*f)(char *bin_file, void *arg))
{
    char    *line = NULL;
    size_t   n    = 0;
    ssize_t  r    = getline(&line, &n, fp);;
    if (r == -1) {
        if (errno != 0)
            err(1, "getline");
        return -1;
    }
    int64_t value;
    char *type = test_parse_line(line, &value);
    if (type == NULL)
        err(1, "test_parse_line");

    data->value = value;
    data->type_changed = 0;
    if (strcmp(type, data->type) != 0) {
        data->type_changed = 1;
        data->pos = 0;
        snprintf(data->type, sizeof(data->type), "%s", type);
        ssize_t index = find_type(type, tests);
        if (index < 0) {
            data->will_be_tested = 0;
            goto out;
        }
        data->will_be_tested = 1;
        data->index = index;

        char *bin_file = test_bin_file(type);
        f(bin_file, arg);
        free(bin_file);
    }
out:
    free(line);
    return 0;
}
