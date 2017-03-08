#include <sys/types.h>
#include <sys/socket.h>

#include <err.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <bin_prot/rpc.h>
#include <bin_prot/type_class.h>

static int
connect_to_server(const char *host, uint16_t port,
                  const struct sockaddr *interface, socklen_t interface_len)
{
    int ret;
    int fd;
    struct addrinfo hints, *ai, *p;
    char port_string[6];

    snprintf(port_string, 6, "%u", port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    ret = getaddrinfo(NULL, port_string, &hints, &ai);
    if (ret == -1)
        return -1;

    for (p = ai; p != NULL; p = p->ai_next) {
        fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd == -1)
            continue;

        ret = connect(fd, p->ai_addr, p->ai_addrlen);
        if (ret == -1) {
            close(fd);
            continue;
        }

        if (interface != NULL) {
            ret = bind(fd, interface, interface_len);
            if (ret == -1) {
                close(fd);
                continue;
            }
        }

        break;
    }

    freeaddrinfo(ai);

    if (p == NULL)
        return -1;

    return fd;
}

int
main(void)
{
    int ret;
    struct bin_rpc rpc;
    struct bin_rpc_connection *conn;

	int fd = connect_to_server("localhost", 8124, NULL, 0);
    if (fd == -1)
        err(1, "connect_to_server");

    conn = bin_rpc_client(fd, "my client");
    if (conn == NULL)
        errx(1, "bin_rpc_client");

    rpc.tag = "hello-world";
    rpc.version = 0;
    rpc.bin_query = &bin_string;
    rpc.bin_response = &bin_string;

    char *s;
    ret = bin_rpc_dispatch(&rpc, conn, "Hello", &s);
    if (ret == -1)
        errx(1, "bin_rpc_dispatch");

    printf("hello-world response: %s\n", s);
    free(s);

    rpc.tag = "incr";
    rpc.version = 0;
    rpc.bin_query = &bin_int;
    rpc.bin_response = &bin_int;

    long i = 41;
    ret = bin_rpc_dispatch(&rpc, conn, &i, &i);
    if (ret == -1)
        errx(1, "bin_rpc_dispatch");

    printf("incr response: %ld\n", i);

    bin_rpc_connection_free(conn);

    return 0;
}
