#include <stdlib.h>

#include <bin_prot/type_class.h>
#include <bin_prot/rpc.h>

void *
bin_dump(int header, struct bin_type_class_writer *writer,
         void *v, size_t *buf_len)
{
    char *buf;
    size_t pos, pos_len, v_len;

    v_len = writer->size(v, writer->size_data);
    if (header) {
        size_t tot_len = v_len + BIN_RPC_HEADER_LENGTH;
        buf = malloc(tot_len);
        pos = bin_write_int_64bit(buf, 0, v_len);
        pos_len = pos + v_len;
    } else {
        buf = malloc(v_len);
        pos = 0;
        pos_len = v_len;
    }
    pos = writer->write(buf, pos, v, writer->write_data);
    if (pos != pos_len) {
        free(buf);
        return NULL; /* size changed during writing */
    }

    *buf_len = pos_len;

    return buf;
}
