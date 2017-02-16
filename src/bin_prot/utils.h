#ifndef BIN_PROT_UTILS_H
#define BIN_PROT_UTILS_H

#include <bin_prot/type_class.h>

void *bin_dump(int header, struct bin_type_class_writer *writer,
               void *v, size_t *v_len);

#endif
