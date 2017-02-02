CFLAGS = -Wall -g
PREFIX = /usr/local

all: build

build: build_setup common read write size
	cd _build/src && \
		gcc $(CFLAGS) -shared -o libbin_prot.so common.o read.o write.o

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

install: build
	mkdir -p $(PREFIX)/include
	mkdir -p $(PREFIX)/lib
	install -m644 _build/src/bin_prot.h $(PREFIX)/include
	install -m644 _build/src/libbin_prot.so $(PREFIX)/lib

uninstall:
	rm -f $(PREFIX)/include/bin_prot.h
	rm -f $(PREFIX)/lib/libbin_prot.so

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

test_build_c_int_reader: build test_setup test_build_c_base test/int_reader.c
	cd _build/test &&                \
		gcc $(CFLAGS) $(TEST_CFLAGS)   \
			-o c_int_reader              \
			test.o                       \
			int_reader.c $(TEST_LDFLAGS)

test_build_c_int_writer: build test_setup test_build_c_base test/int_writer.c
	cd _build/test &&                \
		gcc $(CFLAGS) $(TEST_CFLAGS)   \
			-o c_int_writer              \
			test.o                       \
			int_writer.c $(TEST_LDFLAGS)

test_build_c_base: test_setup test/test.c
	cd _build/test && \
		gcc $(CFLAGS) $(TEST_CFLAGS) -o test.o -c test.c

.PHONY: test build
