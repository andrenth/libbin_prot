# libbin\_prot

## Introduction

This library is a C port of OCaml's [bin\_prot](https://github.com/janestreet/bin_prot)
serialization library. Its main purpose is to allow the communication of OCaml
programs with programs written in other languages, which can simply bind to
libbin\_prot instead of reimplementing serializating over and over again.

## Installation

You can build php-bin\_prot by typing `make` and install it with `make install`
(likely with `sudo` in front of the latter).

You can also build a `deb` package with `make deb`.

## Usage

Serialization functions are named following the OCaml types. Their signatures
are outlined below.

### Readers

Readers are of the form below:

```c
#include <bin_prot/read.h>

int bin_read_<ocaml type>(char *buf, size_t *pos, <c type> *res);
```

Read functions read a value of the given type from buffer `buf` at position
`*pos`, storing the result in `res` and updating the position in `pos`. They
return `0` on success, and `1` on error.

Note that some reader functions (`bin_read_string()`, `bin_read_digest()`, and
the `bin_read_<type>_array()` functions) allocate memory on the heap, so `res`
must be freed after it's used.

### Writers

Writers are of the form below:

```c
#include <bin_prot/write.h>

size_t bin_write_<ocaml type>(char *buf, size_t pos, <c type> val);
```

Writer functions write the value `val` into buffer `buf` at position `pos`. The
updated position is returned.

### Sizers

Sizers live in the `\bin_prot\size` namespace and are of the form below:

```c
#include <bin_prot/size.h>

size_t bin_size_<ocaml type>(<c type> *val);
```

Sizer functions return the serialized size in bytes of the value `val`.

### Type classes

Type classes are simply structs with fields containing readers, writers and
sizers.

Instances of type classes for the built-in types are provided as global
variables in `bin_prot/type_class.h`.

### Higher order readers, writers and type classes.

Serialization functions for container types such as arrays, tuples or the option
type take one or more extra arguments which specify readers, writers or sizers
for the elements of the container.

For example, for an option type, the reader function looks like this:

```c
int bin_read_option(bin_reader bin_read_el, void *buf, size_t *pos, void *res);
```

The type `bin_reader` is a pointer to a function with the signature of a
regular (non-higher order) reader.

So to read an option type that possibly contains an OCaml float value, one
would call

```c
double res;
ret = bin_read_option((bin_reader)bin_read_float, buf, pos, &res);
```

This function returns `0` for the `Some` case, in which case the float value
will be stored in `res`, `1` for the `None` case and `-1` in case of error.

### RPC

The library supports simple RPC clients. The functions are described below.
Currently the library uses blocking I/O. Exploring asynchronous I/O options,
maybe with [libuv](https://github.com/libuv/libuv) or some similar library,
could be interesting future work.

```c
#include <bin_prot/rpc.h>
#include <bin_prot/type_class.h>

struct bin_rpc_connection *bin_rpc_client(int fd, const char *description);

struct bin_rpc {
    char *tag;
    long  version;
    struct bin_type_class *bin_query;
    struct bin_type_class *bin_response;
};

int bin_rpc_dispatch(struct bin_rpc *rpc, struct bin_rpc_connection *conn,
                     void *data, void *res);

void bin_rpc_connection_free(struct bin_rpc_connection *conn);
```

One starts by calling `bin_rpc_client()`, which expects a `connect()`ed socket
as its first parameter. Then the `bin_rpc` struct must be filled with the tag
and version that identify the RPC, and the type classes that describe query and
response serialization. The `bin_prot/type_class.h` header includes some
pre-defined type classes for basic types.

The `bin_rpc_dispatch()` function builds an RPC query based on the `data`
parameter. It returns 0 on success, in which case the `res` will point to the
RPC response. Remember that some reader functions, like `bin_read_string()`,
allocate memory on the heap, so it must be freed after it is used.

Finally, `bin_rpc_connection_free()` frees the memory allocated by
`bin_rpc_client`.

The `examples` directory contains a simple OCaml RPC server and a C RPC client
using `libbin_prot`.
