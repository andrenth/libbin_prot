#include <sys/types.h>
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <bin_prot.h>

#include "test.h"

struct read_test {
    char   *name;
    reader  read;
    void   *buf;
};

struct read_line_data {
    void    *buffer;
    char     type[32];
    int64_t  value;
    size_t   index;
    size_t   pos;
    int      will_be_tested;
    int      type_changed;
};

struct read_test readers[] = {
    { "int",             (reader)bin_read_int,             NULL },
    { "int32",           (reader)bin_read_int32,           NULL },
    { "int64",           (reader)bin_read_int64,           NULL },
    { "int_16bit",       (reader)bin_read_int_16bit,       NULL },
    { "int_32bit",       (reader)bin_read_int_32bit,       NULL },
    { "int_64bit",       (reader)bin_read_int_64bit,       NULL },
    { "int64_bits",      (reader)bin_read_int64_bits,      NULL },
    { "network16_int",   (reader)bin_read_network16_int,   NULL },
    { "network16_int16", (reader)bin_read_network16_int16, NULL },
    { "network32_int",   (reader)bin_read_network32_int,   NULL },
    { "network32_int32", (reader)bin_read_network32_int32, NULL },
    { "network64_int",   (reader)bin_read_network64_int,   NULL },
    { "network64_int64", (reader)bin_read_network64_int64, NULL },
    { "nat0",            (reader)bin_read_nat0,            NULL },
    { "variant_int",     (reader)bin_read_variant_int,     NULL },
    { NULL,              NULL,                             NULL },
};

void
read_int(int64_t expected, struct read_test *r, size_t *pos)
{
    int res;
    int64_t actual;

    res = r->read(r->buf, pos, &actual);
    if (res == -1)
        errx(1, "read_int: %s", r->name);

    if (expected >= INT16_MIN && expected <= INT16_MAX)
        actual = (int16_t)actual;
    else if (expected >= INT32_MIN && expected <= INT32_MAX)
        actual = (int32_t)actual;

    if (actual != expected)
        printf("    expected %ld, got %ld\n", expected, actual);
}

static ssize_t
file_size(const char *file_name)
{
    int ret;
    struct stat st;

    ret = stat(file_name, &st);
    if (ret == -1)
        err(1, "stat: %s", file_name);

    return st.st_size;
}

void *
read_bin(char *bin_file)
{
    ssize_t len = file_size(bin_file);
    ssize_t n   = 0;

    int fd = open(bin_file, O_RDONLY);
    if (fd == -1)
        err(1, "open");

    void *buf = malloc(len);
    while (n < len)
        n += read(fd, buf + n, len - n);

    close(fd);
    return buf;
}

ssize_t
find_reader(char *type, struct read_test *readers)
{
    ssize_t i;
    for (i = 0; readers[i].name != NULL; i++) {
        if (strcmp(type, readers[i].name) != 0)
            continue;
        return i;
    }
    return -1;
}

int
read_line(FILE *fp, struct read_line_data *data)
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
        ssize_t index = find_reader(type, readers);
        if (index < 0) {
            data->will_be_tested = 0;
            goto out;
        }
        data->will_be_tested = 1;
        data->index = index;

        char *bin_file = test_bin_file(type);
        data->buffer = read_bin(bin_file);
        free(bin_file);
    }
out:
    free(line);
    return 0;
}

int
main(void)
{
    FILE *fp;

    fp = fopen(TEST_EXPECTATIONS_FILE, "r");
    if (fp == NULL)
        err(1, "fopen");

    printf("[-] Testing reader\n");

    for (;;) {
        struct read_line_data data;
        int r = read_line(fp, &data);
        if (r == -1)
            break; /* eof */
        if (data.type_changed) {
            if (!data.will_be_tested) {
                printf("  ! %s will not be tested\n", data.type);
                continue;
            }
            printf("  * %s\n", data.type);
        }
        if (data.will_be_tested) {
            struct read_test *reader;
            reader = &readers[data.index];
            reader->buf = data.buffer;
            read_int(data.value, reader, &data.pos);
            //free(reader->buf);
        }
    }
    fclose(fp);
}
