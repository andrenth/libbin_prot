#include <bin_prot/type_class.h>

#define DEFINE_TYPE_CLASS(name, type)                                      \
static size_t                                                              \
bin_type_class_size_##name(type *v, void *_data)                           \
{                                                                          \
    return bin_size_##name(v);                                             \
}                                                                          \
static size_t                                                              \
bin_type_class_write_##name(void *buf, size_t pos, void *v, void *_data)   \
{                                                                          \
    return bin_write_##name(buf, pos, *(type *)v);                         \
}                                                                          \
static int                                                                 \
bin_type_class_read_##name(void *buf, size_t *pos, type *res, void *_data) \
{                                                                          \
    return bin_read_##name(buf, pos, res);                                 \
}                                                                          \
struct bin_type_class_writer name##_writer = {                             \
    .size  = (bin_type_class_sizer_ex)bin_type_class_size_##name,          \
    .write = (bin_type_class_writer_ex)bin_type_class_write_##name,        \
};                                                                         \
struct bin_type_class_reader name##_reader = {                             \
    .read = (bin_type_class_reader_ex)bin_type_class_read_##name,          \
};                                                                         \
struct bin_type_class bin_##name = {                                       \
    .writer = &name##_writer,                                              \
    .reader = &name##_reader,                                              \
};

DEFINE_TYPE_CLASS(unit, void *)
DEFINE_TYPE_CLASS(bool, int)
DEFINE_TYPE_CLASS(char, unsigned char)

DEFINE_TYPE_CLASS(nat0,  unsigned long)
DEFINE_TYPE_CLASS(int,   long)
DEFINE_TYPE_CLASS(int32, int32_t)
DEFINE_TYPE_CLASS(int64, int64_t)

DEFINE_TYPE_CLASS(int_8bit,  long)
DEFINE_TYPE_CLASS(int_16bit, long)
DEFINE_TYPE_CLASS(int_32bit, long)
DEFINE_TYPE_CLASS(int_64bit, long)

DEFINE_TYPE_CLASS(network16_int, long)
DEFINE_TYPE_CLASS(network32_int, long)
DEFINE_TYPE_CLASS(network64_int, long)

DEFINE_TYPE_CLASS(network16_int16, int16_t)
DEFINE_TYPE_CLASS(network32_int32, int32_t)
DEFINE_TYPE_CLASS(network64_int64, int64_t)

DEFINE_TYPE_CLASS(variant_int, long)

DEFINE_TYPE_CLASS(float, double)

static size_t
bin_type_class_size_string(char *v, void *_data)
{
    return bin_size_string(v);
}
static size_t
bin_type_class_write_string(void *buf, size_t pos, char *v, void *_data)
{
    return bin_write_string(buf, pos, v);
}
static int
bin_type_class_read_string(void *buf, size_t *pos, char **res, void *_data)
{
    return bin_read_string(buf, pos, res);
}
struct bin_type_class_writer string_writer = {
    .size  = (bin_type_class_sizer_ex)bin_type_class_size_string,
    .write = (bin_type_class_writer_ex)bin_type_class_write_string,
};
struct bin_type_class_reader string_reader = {
    .read = (bin_type_class_reader_ex)bin_type_class_read_string,
};
struct bin_type_class bin_string = {
    .writer = &string_writer,
    .reader = &string_reader,
};

//DEFINE_TYPE_CLASS(string, char *)
