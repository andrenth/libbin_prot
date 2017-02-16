#ifndef BIN_TYPE_CLASS_H
#define BIN_TYPE_CLASS_H

#include <bin_prot/read.h>
#include <bin_prot/write.h>
#include <bin_prot/size.h>

struct bin_type_class_writer {
    bin_sizer  size;
    bin_writer write;
};

struct bin_type_class_reader {
    bin_reader read;
    /*XXX vtag_reader vtag_read; */
};

struct bin_type_class {
    struct bin_type_class_writer *writer;
    struct bin_type_class_reader *reader;
};

extern struct bin_type_class bin_nat0;
extern struct bin_type_class bin_string;

extern struct bin_type_class bin_unit;
extern struct bin_type_class bin_bool;
extern struct bin_type_class bin_char;

extern struct bin_type_class bin_nat0;
extern struct bin_type_class bin_int;
extern struct bin_type_class bin_int32;
extern struct bin_type_class bin_int64;

extern struct bin_type_class bin_int_8bit;
extern struct bin_type_class bin_int_16bit;
extern struct bin_type_class bin_int_32bit;
extern struct bin_type_class bin_int_64bit;

extern struct bin_type_class bin_network16_int;
extern struct bin_type_class bin_network32_int;
extern struct bin_type_class bin_network64_int;

extern struct bin_type_class bin_variant_int;

extern struct bin_type_class bin_float;

extern struct bin_type_class bin_string;

#endif
