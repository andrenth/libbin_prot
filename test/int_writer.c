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
};

struct write_line_data {
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
write_int(int64_t i, struct write_test *w, FILE *fp)
{
    size_t pos = w->write(buf, 0, (void *)i);
    fwrite(buf, pos, 1, fp);
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

void
set_fp(char *bin_file, void *arg)
{
    FILE **fp = (FILE **)arg;
    if (*fp != NULL)
        fclose(*fp);
    *fp = fopen(bin_file, "w");
    if (*fp == NULL)
        err(1, "fopen: %s", bin_file);
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
        FILE   *bin_fp;
        struct  line_data data;
        int r = test_read_line(fp, &data, (struct test *)writers, &bin_fp, set_fp);
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
            write_int(data.value, writer, bin_fp);
        }
    }
}
