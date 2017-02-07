MAJOR  = 1
VER    = 1.0
CFLAGS = -Wall -g
PREFIX = /usr/local

define PKG_CONFIG
prefix=$(PREFIX)
exec_prefix=$(PREFIX)/bin
libdir=$(PREFIX)/lib
includedir=$(PREFIX)/include

Name: libbin_prot
Description: bin_prot implementation in C
Version: $(VER)

Libs: -L$${libdir} -lbin_prot
Cflags: -I$${includedir}
endef

all: shared static

shared: build_setup common read write size
	cd _build/src &&                        \
		gcc $(CFLAGS) -shared                 \
			-Wl,-soname,libbin_prot.so.$(MAJOR) \
			-o libbin_prot.so.$(MAJOR).$(VER)   \
			common.o read.o write.o size.o

static: build_setup
	cd _build/src && \
		ar rcs libbin_prot.a common.o read.o write.o size.o && \
		ranlib libbin_prot.a

build_setup:
	mkdir -p _build/src
	cp -a src/* _build/src

common: src/common.c build_setup
	cd _build/src && gcc $(CFLAGS) -fPIC -o common.o -c common.c

read: src/read.c build_setup common
	cd _build/src && gcc $(CFLAGS) -fPIC -o read.o -c read.c

write: src/write.c build_setup common
	cd _build/src && gcc $(CFLAGS) -fPIC -o write.o -c write.c

size: src/size.c build_setup common
	cd _build/src && gcc $(CFLAGS) -fPIC -o size.o -c size.c

export PKG_CONFIG
pkgconfig:
	@echo "$$PKG_CONFIG" > _build/libbin_prot.pc

install: shared static pkgconfig
	mkdir -p $(PREFIX)/include
	install -m0644 _build/src/bin_prot.h $(PREFIX)/include
	mkdir -p $(PREFIX)/lib
	strip _build/src/libbin_prot.so.$(MAJOR).$(VER)
	install -m0644 _build/src/libbin_prot.so.$(MAJOR).$(VER) $(PREFIX)/lib
	ln -sf libbin_prot.so.$(MAJOR).$(VER) $(PREFIX)/lib/libbin_prot.so
	ln -sf libbin_prot.so.$(MAJOR).$(VER) $(PREFIX)/lib/libbin_prot.so.$(MAJOR)
	install -m0644 _build/libbin_prot.pc $(PREFIX)/lib/pkgconfig

uninstall:
	rm -f $(PREFIX)/include/bin_prot.h
	rm -f $(PREFIX)/lib/libbin_prot.so
	rm -f $(PREFIX)/lib/libbin_prot.so.$(MAJOR)
	rm -f $(PREFIX)/lib/libbin_prot.so.$(MAJOR).$(VER)

clean:
	rm -rf _build

TEST_CFLAGS = -I../src -L../src
TEST_LDFLAGS = -lbin_prot

test: test_sanity test_c_int_reader test_c_int_writer

test_sanity: test_build_ocaml_int_reader test_build_ocaml_int_writer
	./_build/test/ocaml_int_writer && ./_build/test/ocaml_int_reader

test_c_int_reader: test_setup test_build_ocaml_int_writer test_build_c_int_reader
	./_build/test/ocaml_int_writer && \
		LD_LIBRARY_PATH=_build/src ./_build/test/c_int_reader

test_c_int_writer: test_setup test_build_c_int_writer test_build_ocaml_int_reader
	LD_LIBRARY_PATH=_build/src ./_build/test/c_int_writer && \
		./_build/test/ocaml_int_reader

test_setup:
	mkdir -p _build/test
	mkdir -p _build/bin
	cp -a test/* _build/test

test_build_ocaml_int_reader: test_setup test/int_reader.ml
	cd _build/test &&                        \
		ocamlfind ocamlopt -o ocaml_int_reader \
			-thread                              \
			-linkpkg                             \
			-package core,bin_prot,unix          \
			test.ml                              \
			int_reader.ml

test_build_ocaml_int_writer: test_setup test/int_writer.ml
	cd _build/test &&                        \
		ocamlfind ocamlopt -o ocaml_int_writer \
			-thread                              \
			-linkpkg                             \
			-package core,bin_prot,unix          \
			test.ml                              \
			int_writer.ml

test_build_c_int_reader: shared static test_setup test_build_c_base test/int_reader.c
	cd _build/test &&                \
		gcc $(CFLAGS) $(TEST_CFLAGS)   \
			-o c_int_reader              \
			test.o                       \
			int_reader.c $(TEST_LDFLAGS)

test_build_c_int_writer: shared static test_setup test_build_c_base test/int_writer.c
	cd _build/test &&                \
		gcc $(CFLAGS) $(TEST_CFLAGS)   \
			-o c_int_writer              \
			test.o                       \
			int_writer.c $(TEST_LDFLAGS)

test_build_c_base: test_setup test/test.c
	cd _build/test && \
		gcc $(CFLAGS) $(TEST_CFLAGS) -o test.o -c test.c

.PHONY: test build
