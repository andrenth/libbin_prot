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
