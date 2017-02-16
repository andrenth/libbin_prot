#include <bin_prot/type_class.h>

#define DEFINE_TYPE_CLASS(name)                 \
struct bin_type_class_writer name##_writer = {  \
    .size  = (bin_sizer)bin_size_##name,        \
    .write = (bin_writer)bin_write_##name,      \
};                                              \
struct bin_type_class_reader name##_reader = {  \
    .read = (bin_reader)bin_read_##name,        \
};                                              \
struct bin_type_class bin_type_class_##name = { \
    .writer = &name##_writer,                   \
    .reader = &name##_reader,                   \
};

DEFINE_TYPE_CLASS(unit)
DEFINE_TYPE_CLASS(bool)
DEFINE_TYPE_CLASS(char)

DEFINE_TYPE_CLASS(nat0)
DEFINE_TYPE_CLASS(int)
DEFINE_TYPE_CLASS(int32)
DEFINE_TYPE_CLASS(int64)

DEFINE_TYPE_CLASS(int_8bit)
DEFINE_TYPE_CLASS(int_16bit)
DEFINE_TYPE_CLASS(int_32bit)
DEFINE_TYPE_CLASS(int_64bit)

DEFINE_TYPE_CLASS(network16_int)
DEFINE_TYPE_CLASS(network32_int)
DEFINE_TYPE_CLASS(network64_int)

DEFINE_TYPE_CLASS(variant_int)

DEFINE_TYPE_CLASS(float)

DEFINE_TYPE_CLASS(string)
