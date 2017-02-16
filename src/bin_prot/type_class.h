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

extern struct bin_type_class bin_type_class_nat0;
extern struct bin_type_class bin_type_class_string;

extern struct bin_type_class bin_type_class_unit;
extern struct bin_type_class bin_type_class_bool;
extern struct bin_type_class bin_type_class_char;

extern struct bin_type_class bin_type_class_nat0;
extern struct bin_type_class bin_type_class_int;
extern struct bin_type_class bin_type_class_int32;
extern struct bin_type_class bin_type_class_int64;

extern struct bin_type_class bin_type_class_int_8bit;
extern struct bin_type_class bin_type_class_int_16bit;
extern struct bin_type_class bin_type_class_int_32bit;
extern struct bin_type_class bin_type_class_int_64bit;

extern struct bin_type_class bin_type_class_network16_int;
extern struct bin_type_class bin_type_class_network32_int;
extern struct bin_type_class bin_type_class_network64_int;

extern struct bin_type_class bin_type_class_variant_int;

extern struct bin_type_class bin_type_class_float;

extern struct bin_type_class bin_type_class_string;

#endif
