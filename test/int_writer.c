#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bin_prot.h>

#include "test.h"

struct write_test {
    char   *name;
    writer  write;
    FILE   *fp;
};

struct write_line_data {
    FILE    *fp;
    char     type[32];
    int64_t  value;
    size_t   index;
    size_t   pos;
    int      will_be_tested;
    int      type_changed;
};

char buf[32];

struct write_test writers[] = {
    { "int",             (writer)bin_write_int,             NULL },
    { "int32",           (writer)bin_write_int32,           NULL },
    { "int64",           (writer)bin_write_int64,           NULL },
    { "int_16bit",       (writer)bin_write_int_16bit,       NULL },
    { "int_32bit",       (writer)bin_write_int_32bit,       NULL },
    { "int_64bit",       (writer)bin_write_int_64bit,       NULL },
    { "int64_bits",      (writer)bin_write_int64_bits,      NULL },
    { "network16_int",   (writer)bin_write_network16_int,   NULL },
    { "network16_int16", (writer)bin_write_network16_int16, NULL },
    { "network32_int",   (writer)bin_write_network32_int,   NULL },
    { "network32_int32", (writer)bin_write_network32_int32, NULL },
    { "network64_int",   (writer)bin_write_network64_int,   NULL },
    { "network64_int64", (writer)bin_write_network64_int64, NULL },
    { "nat0",            (writer)bin_write_nat0,            NULL },
    { "variant_int",     (writer)bin_write_variant_int,     NULL },
    { NULL,              NULL,                              NULL },
};

void
write_int(int64_t i, struct write_test *w)
{
    size_t pos = w->write(buf, 0, (void *)i);
    fwrite(buf, pos, 1, w->fp);
}

ssize_t
find_writer(char *type, struct write_test *writers)
{
    ssize_t i;
    for (i = 0; writers[i].name != NULL; i++) {
        if (strcmp(type, writers[i].name) != 0)
            continue;
        return i;
    }
    return -1;
}

int
read_line(FILE *fp, struct write_line_data *data)
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
        ssize_t index = find_writer(type, writers);
        if (index < 0) {
            data->will_be_tested = 0;
            goto out;
        }
        data->will_be_tested = 1;
        data->index = index;

        char *bin_file = test_bin_file(type);
        data->fp = fopen(bin_file, "w");
        if (data->fp == NULL)
            err(1, "fopen: %s", bin_file);
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

    printf("[-] Testing writer\n");

    for (;;) {
        struct write_line_data data;
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
            struct write_test *writer;
            writer = &writers[data.index];
            writer->fp = data.fp;
            write_int(data.value, writer);
            //fclose(data.fp);
        }
    }
    fclose(fp);
}

#if 0
int
main(void)
{
    FILE *fp;

    fp = fopen(TEST_EXPECTATIONS_FILE, "r");
    if (fp == NULL)
        err(1, "fopen");

    char cur_type[16] = "";
    size_t cur_type_idx = 0;
    for (;;) {
        char    *line = NULL;
        size_t   n    = 0;
        ssize_t  r    = getline(&line, &n, fp);;
        if (r == -1) {
            if (errno != 0)
                err(1, "getline");
            break;
        }
        int64_t i64;
        char *type = test_parse_line(line, &i64);
        if (type == NULL)
            err(1, "test_parse_line");

        if (strcmp(type, cur_type) != 0) {
            int i;
            for (i = 0; writers[i].name != NULL; i++) {
                if (strcmp(type, writers[i].name) != 0)
                    continue;
                cur_type_idx = i;
                break;
            }
            char *bin_file = test_bin_file(type);
            FILE *fp = fopen(bin_file, "w");
            free(bin_file);
            if (fp == NULL)
                continue; /* ignore unsupported type */
            writers[cur_type_idx].fp = fp;
            snprintf(cur_type, sizeof(cur_type), "%s", type);
            printf("testing %s\n", type);
        }
        write_int(i64, &writers[cur_type_idx]);
        free(line);
    }

    fclose(fp);
}
#endif
