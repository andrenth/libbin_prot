#include <sys/types.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <bin_prot/read.h>
#include <bin_prot/write.h>
#include <bin_prot/type_class.h>
#include <bin_prot/rpc.h>
#include <bin_prot/utils.h>

#include "common.h"

struct query {
    char    *tag;
    long     version;
    int64_t  id; /* int63_t */
    void    *data;
};

struct response {
    int64_t  id; /* int63_t */
    void    *data;
};

struct transport {
    int   fd;
    void *buf;
};

struct bin_rpc_connection {
    struct transport *transport;
    const char       *description;
};

typedef enum {
    MSG_HEARTBEAT,
    MSG_QUERY,
    MSG_RESPONSE,
} message_type;

struct message {
    message_type  type;
    void         *data;
};

/*
 * Add an extra length field to some data.
 */

struct with_length_args {
    struct bin_type_class_writer *writer;
    void                         *data;
};

static size_t
write_with_length(void *buf, size_t pos, void *arg, void *_unused)
{
    struct with_length_args *args = (struct with_length_args *)arg;
    struct bin_type_class_writer *writer = args->writer;
    size_t len = writer->size(args->data, writer->size_data);

    pos = bin_write_nat0(buf, pos, len);
    pos = writer->write(buf, pos, args->data, writer->write_data);
    return pos;
}

static size_t
size_with_length(void *arg, void *_unused)
{
    struct with_length_args *args = (struct with_length_args *)arg;
    struct bin_type_class_writer *writer = args->writer;
    size_t len = writer->size(args->data, writer->size_data);

    return bin_size_nat0(&len) + len;
}

typedef enum {
    RESULT_OK,
    RESULT_ERROR,
} result;

static int
bin_read_response(struct bin_type_class_reader *data_reader,
                  int64_t *id, void *data, void *buf, size_t *pos)
{
    int ret;
    unsigned long len;
    long ok_or_error;

    ret = bin_read_int64(buf, pos, id);
    if (ret == -1)
        return -1;

    /* Rpc_result.t is Result.t */
    ret = bin_read_int_8bit(buf, pos, &ok_or_error);
    if (ret == -1)
        return -1;
    if (ok_or_error == RESULT_ERROR)
        return -1;

    /* Read "with length" extra length field. XXX is this always present? */
    ret = bin_read_nat0(buf, pos, &len);
    if (ret == -1)
        return -1;

    ret = data_reader->read(buf, pos, data, data_reader->read_data);
    if (ret == -1)
        return -1;

    return 0;
}

static int64_t
query_id(void)
{
    static int64_t id = -1;
    return ++id;
}

typedef enum {
    RESP_ERROR = -1,
    RESP_KEEP,
    RESP_WAIT,
    RESP_REMOVE,
    RESP_REMOVE_AND_WAIT,
} response_code;

static int
connection_closed(struct bin_rpc_connection *conn)
{
    return conn->transport->fd == -1;
}

static int
send_n(int fd, void *buf, size_t len)
{
    size_t n;
    ssize_t k;

    for (n = 0, k = 0; n < len; n += k) {
        k = send(fd, buf + k, len - n, 0);
        if (k == -1)
            return -1;
    }
    return 0;
}

static int
send_bin_prot(struct bin_rpc_connection *conn,
              struct bin_type_class_writer *writer,
              void *arg)
{
    void  *buf;
    size_t len;
    int    ret;

    buf = bin_dump(1, writer, arg, &len);
    if (buf == NULL)
        return -1;

    ret = send_n(conn->transport->fd, buf, len);
    free(buf);
    return ret;
}

struct read_msg_args {
    struct message *msg;
    struct bin_type_class_reader *reader;
};

static int
bin_read_message(void *buf, size_t *pos, void *arg, void *_unused)
{
    int ret;
    long type;
    struct read_msg_args *args = (struct read_msg_args *)arg;
    struct message *msg = args->msg;
    struct bin_type_class_reader *reader = args->reader;

    ret = bin_read_int_8bit(buf, pos, &type);
    if (ret == -1)
        return -1;

    msg->type = type;

    switch (type) {
    case MSG_QUERY:
        /* Can a client receive queries? */
        return -1;
    case MSG_RESPONSE: {
        int64_t id;
        ret = bin_read_response(reader, &id, msg->data, buf, pos);

        if (ret == -1) {
            // XXX string? free(resp.data);
            return -1;
        }
        return 0;
    }
    case MSG_HEARTBEAT:
        return 0;
    }

    /* not reached */
    return 0;
}

struct message_args {
    struct message               *msg;
    struct bin_type_class_writer *data_writer;
};

static size_t
bin_write_message(void *buf, size_t pos, void *arg, void *_unused)
{
    struct message_args *args = (struct message_args *)arg;
    struct message *msg = args->msg;
    struct bin_type_class_writer *writer = args->data_writer;

    pos = bin_write_int_8bit(buf, pos, msg->type);
    switch (msg->type) {
    case MSG_QUERY:
    case MSG_RESPONSE:
        return writer->write(buf, pos, msg->data, writer->write_data);
    case MSG_HEARTBEAT:
        return pos;
    }
    /* not reached */
    return 0;
}

static size_t
bin_size_message(void *arg, void *_unused)
{
    struct message_args *args = (struct message_args *)arg;
    struct message *msg = args->msg;
    struct bin_type_class_writer *data_writer = args->data_writer;

    switch (msg->type) {
    case MSG_QUERY:
    case MSG_RESPONSE:
        return 1 + data_writer->size(msg->data, data_writer->size_data);
    case MSG_HEARTBEAT:
        return 1;
    }
    /* not reached */
    return 0;
}

struct query_writer_args {
    struct query                 *query;
    struct bin_type_class_writer *writer;
    void                         *writer_args;
};

static size_t
bin_write_query(void *buf, size_t pos, void *arg, void *_unused)
{
    struct query_writer_args *args = (struct query_writer_args *)arg;
    struct query *query = args->query;
    struct bin_type_class_writer *writer = args->writer;

    pos = bin_write_string(buf, pos, query->tag);
    pos = bin_write_int(buf, pos, query->version);
    if (arch_sixtyfour())
        pos = bin_write_int(buf, pos, query->id);
    else
        pos = bin_write_int64(buf, pos, query->id);
    pos = writer->write(buf, pos, args->writer_args, writer->write_data);

    return pos;
}

static size_t
bin_size_query(void *arg, void *_unused)
{
    struct query_writer_args *args = (struct query_writer_args *)arg;
    struct query *query = args->query;
    struct bin_type_class_writer *writer = args->writer;
    size_t size = 0;

    size += bin_size_string(query->tag);
    size += bin_size_int(&query->version);
    if (arch_sixtyfour())
        size += bin_size_int(&query->id);
    else
        size += bin_size_int64(&query->id);
    size += writer->size(args->writer_args, writer->write_data);

    return size;
}

static int
dispatch_raw(struct bin_rpc_connection *conn,
             struct bin_type_class_writer *writer,
             struct query *query)
{
    struct message msg;

    struct bin_type_class_writer writer_with_length;
    struct with_length_args      writer_with_length_args;

    struct bin_type_class_writer query_writer;
    struct query_writer_args     query_writer_args;

    struct bin_type_class_writer msg_writer;
    struct message_args          msg_writer_args;

    if (connection_closed(conn))
        return -1;

    /* Prepend the query bin_prot with its length. */
    writer_with_length.write = write_with_length;
    writer_with_length.size  = size_with_length;

    writer_with_length_args.writer = writer;
    writer_with_length_args.data   = query->data;

    /* Setup a query writer. */
    query_writer.size  = bin_size_query;
    query_writer.write = bin_write_query;

    /* Use the wrapped writer. */
    query_writer_args.query       = query;
    query_writer_args.writer      = &writer_with_length;
    query_writer_args.writer_args = &writer_with_length_args;

    /* Wrap the query in a message. */
    msg.type = MSG_QUERY;
    msg.data = &query_writer_args;

    msg_writer.write = bin_write_message;
    msg_writer.size  = bin_size_message;

    msg_writer_args.msg              = &msg;
    msg_writer_args.data_writer      = &query_writer;

    return send_bin_prot(conn, &msg_writer, &msg_writer_args);
    /* XXX Sent/Closed/Message_too_big? */
}

#define BUFFER_SIZE 128 * 1024

static size_t
bin_write_header(void *buf, size_t pos, void *val, void *_unused)
{
    return bin_write_array((bin_writer)bin_write_int, buf, pos, val, 1);
}

static size_t
bin_size_header(void *val, void *_unused)
{
    return bin_size_array((bin_sizer)bin_size_int, &val, 1);
}

struct read_header_args {
    long   *versions;
    size_t  len;
};

static int
bin_read_header(void *buf, size_t *pos, void *arg, void *_unused)
{
    int i, ret;
    long *vs;
    unsigned long len;
    struct read_header_args *args = (struct read_header_args *)arg;

    ret = bin_read_nat0(buf, pos, &len);
    if (ret == -1)
        return -1;

    vs = malloc(len * sizeof(long));
    for (i = 0; i < len; i++) {
        ret = bin_read_int(buf, pos, &vs[i]);
        if (ret == -1) {
            free(vs);
            return -1;
        }
    }

    args->versions = vs;
    args->len = len;

    return 0;
}

#define HEADER_V1 1

typedef enum {
    READ_RESULT_OK,
    READ_RESULT_EOF,
    READ_RESULT_CLOSED,
    READ_RESULT_ERR,
} read_result;

typedef enum {
    HANDSHAKE_RESULT_OK,
    HANDSHAKE_RESULT_NEGOTIATION_FAILED,
    HANDSHAKE_RESULT_UNEXPECTED_VERSION,
} handshake_result;

static int
read_n(int fd, void *buf, size_t n)
{
    int k = 0;

    while (k < n) {
        ssize_t r = read(fd, buf + k, n - k);
        if (r <= 0)
            return r;
        k += r;
    }

    return k;
}

int64_t
header_unsafe_get_payload_length(void *buf, size_t pos)
{
    return unsafe_get64le(buf, pos);
}

typedef enum {
    READ_CALLBACK_STOP,
    READ_CALLBACK_CONTINUE,
    READ_CALLBACK_WAIT,
    READ_CALLBACK_ERR = 99,
} read_callback_result;

typedef read_callback_result (*read_callback)(void *, size_t, size_t, void *);

static void
reset_buffer(struct bin_rpc_connection *conn, size_t *pos, size_t len)
{
    void *buf = conn->transport->buf;
    memmove(buf, buf + *pos, len);
    *pos = 0;
}

static read_result
read_forever(struct bin_rpc_connection *conn,
             read_callback callback, void *callback_arg,
             size_t *pos)
{
    int     ret;
    ssize_t n;
    size_t  payload_pos;
    int64_t payload_len, total_length;

    int    fd      = conn->transport->fd;
    void  *buf     = conn->transport->buf;
    void  *header  = buf + *pos;
    void  *payload = header + BIN_RPC_HEADER_LENGTH;
    size_t length  = BUFFER_SIZE;

    n = read_n(fd, header, BIN_RPC_HEADER_LENGTH);
    if (n == -1)
        return READ_RESULT_CLOSED;
    if (n == 0)
        return READ_RESULT_EOF;

    payload_len  = header_unsafe_get_payload_length(buf, *pos);
    total_length = payload_len + BIN_RPC_HEADER_LENGTH;

    if (total_length <= length) {
        ret = read_n(fd, payload, payload_len);
        if (ret == -1)
            return READ_RESULT_CLOSED;
        if (ret == 0)
            return READ_RESULT_EOF;

        payload_pos = *pos + BIN_RPC_HEADER_LENGTH;
        switch(callback(buf, payload_pos, payload_len, callback_arg)) {
        case READ_CALLBACK_ERR:
            return READ_RESULT_ERR;
        case READ_CALLBACK_STOP:
            return READ_RESULT_OK;
        case READ_CALLBACK_CONTINUE:
        case READ_CALLBACK_WAIT:
            *pos += total_length;
            length = length - total_length;
            if (length - total_length == 0)
                reset_buffer(conn, pos, BUFFER_SIZE);
            return read_forever(conn, callback, callback_arg, pos);
        }
    } else {
        reset_buffer(conn, pos, length);
        return read_forever(conn, callback, callback_arg, pos);
    }
    return READ_RESULT_ERR;
}

struct read_callback_args {
    struct bin_type_class_reader *reader;
    void                         *data;
};

read_callback_result
on_message(void *buf, size_t pos, size_t len, void *arg)
{
    int ret;
    size_t p = pos;
    struct read_callback_args *args = (struct read_callback_args *)arg;
    struct bin_type_class_reader *reader = args->reader;
    struct read_msg_args *msg_args = (struct read_msg_args *)args->data;
    struct message *msg = msg_args->msg;

    ret = reader->read(buf, &p, msg_args, reader->read_data);
    if (ret == -1)
        return READ_CALLBACK_ERR;

    switch(msg->type) {
    case MSG_RESPONSE: {
        return READ_CALLBACK_STOP; /* continue? */
    } case MSG_QUERY:
        return READ_CALLBACK_STOP;
    case MSG_HEARTBEAT:
        return READ_CALLBACK_CONTINUE;
    }

    /* not reached */
    return READ_CALLBACK_ERR;
}

int
bin_rpc_dispatch(struct bin_rpc *rpc, struct bin_rpc_connection *conn,
                 void *data, void *res)
{
    int ret;
    struct query query;
    struct bin_type_class_reader message_reader;
    struct bin_type_class_writer *writer = rpc->bin_query->writer;

    query.tag     = rpc->tag;
    query.version = rpc->version;
    query.id      = query_id();
    query.data    = data;

    ret = dispatch_raw(conn, writer, &query);
    if (ret == -1)
        return -1;

    size_t pos = 0;
    struct message msg;
    struct read_msg_args msg_args;
    struct read_callback_args on_message_args;

    msg.data = res;

    msg_args.msg    = &msg;
    msg_args.reader = rpc->bin_response->reader;

    message_reader.read = bin_read_message;
    message_reader.read_data = rpc->bin_response->reader->read_data;

    on_message_args.reader = &message_reader;
    on_message_args.data = &msg_args;

    ret = read_forever(conn, on_message, &on_message_args, &pos);
    if (ret == -1)
        return -1;

    return 0;
}

static read_callback_result
read_one_message_callback(void *buf, size_t pos, size_t len, void *arg)
{
    int ret;
    size_t p = pos;
    struct read_callback_args *args = (struct read_callback_args *)arg;
    struct bin_type_class_reader *reader = args->reader;

    ret = reader->read(buf, &p, args->data, reader->read_data);
    if (ret == -1)
        return READ_CALLBACK_ERR;
    if (p != pos + len)
        return READ_CALLBACK_ERR;

    return READ_CALLBACK_STOP;
}

static read_result
read_one_message(struct bin_rpc_connection *conn,
                 struct bin_type_class_reader *reader, void *res)
{
    size_t pos;
    struct read_callback_args args;

    args.reader = reader;
    args.data = res;

    pos = 0;
    return read_forever(conn, read_one_message_callback, &args, &pos);
}

static long
header_negotiate_version(long *hdr1, size_t len1, long *hdr2, size_t len2)
{
    size_t i, j;
    long neg = -1;

    for (i = 0; i < len1; i++) {
        for (j = 0; j < len1; j++) {
            if (hdr1[i] == hdr2[j] && hdr1[i] > neg)
                neg = hdr1[i];
        }
    }
    return neg;
}

static handshake_result
do_handshake(struct bin_rpc_connection *conn)
{
    int ret;
    long versions[] = { HEADER_V1 };
    size_t num_ver = sizeof(versions) / sizeof(versions[0]);

    struct bin_type_class_writer header_writer;
    struct bin_type_class_reader header_reader;
    struct read_header_args  read_header_args;

    header_writer.write = bin_write_header;
    header_writer.size  = bin_size_header;

    ret = send_bin_prot(conn, &header_writer, &versions);
    if (ret == -1)
        return -1;

    header_reader.read = bin_read_header;

    ret = read_one_message(conn, &header_reader, &read_header_args);
    if (ret != READ_RESULT_OK)
        return ret;

    ret = header_negotiate_version(read_header_args.versions,
                                   read_header_args.len,
                                   versions, num_ver);
    free(read_header_args.versions);

    switch (ret) {
    case -1:
        return HANDSHAKE_RESULT_NEGOTIATION_FAILED;
    case 1:
        return HANDSHAKE_RESULT_OK;
    default:
        return HANDSHAKE_RESULT_UNEXPECTED_VERSION;
    }
}

void
bin_rpc_connection_free(struct bin_rpc_connection *conn)
{
    close(conn->transport->fd);
    conn->transport->fd = -1;
    free(conn->transport->buf);
    free(conn->transport);
    free(conn);
}

struct bin_rpc_connection *
create_connection(struct transport *transport, const char *description)
{
    int ret;
    struct bin_rpc_connection *conn;

    conn = malloc(sizeof(*conn));
    conn->transport = transport;
    conn->description = description ? description : "<created-directly>";

    ret = do_handshake(conn);
    if (ret != HANDSHAKE_RESULT_OK) {
        bin_rpc_connection_free(conn);
        return NULL;
    }

    return conn;
}

struct bin_rpc_connection *
bin_rpc_client(int fd, const char *description)
{
    struct transport *transport;
    struct bin_rpc_connection *conn;

    transport = malloc(sizeof(*transport));
    transport->fd = fd;
    transport->buf = malloc(BUFFER_SIZE);

    conn = create_connection(transport, description);

    return conn;
}
