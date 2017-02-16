#ifndef BIN_PROT_RPC_H
#define BIN_PROT_RPC_H

#include <sys/types.h>
#include <sys/socket.h>

#include <bin_prot/read.h>
#include <bin_prot/write.h>
#include <bin_prot/size.h>

#include <stdint.h>

#define BIN_RPC_HEADER_LENGTH 8

struct bin_rpc_connection;

struct bin_rpc {
    char *tag;
    long  version;
    struct bin_type_class *bin_query;
    struct bin_type_class *bin_response;
};

struct bin_rpc_connection *bin_rpc_client(int fd, const char *description);

void bin_rpc_connection_free(struct bin_rpc_connection *conn);

int bin_rpc_dispatch(struct bin_rpc *rpc, struct bin_rpc_connection *conn,
                     void *data, void **res);

#endif
