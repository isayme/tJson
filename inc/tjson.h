#ifndef TJSON_H
#define TJSON_H

#include <stdio.h>

typedef enum tjson_value_type_t {
    TJSON_VALUE_TYPE_ERROR = 0,
    TJSON_VALUE_TYPE_NULL,
    TJSON_VALUE_TYPE_STRING,
    TJSON_VALUE_TYPE_NUMBER,
    TJSON_VALUE_TYPE_OBJECT,
    TJSON_VALUE_TYPE_ARRAY,
    TJSON_VALUE_TYPE_BOOLEAN
} tjson_value_type;

typedef struct tjson_object_t tjson_object;
typedef struct tjson_array_t tjson_array;

typedef union tjson_metadata_t tjson_metadata;

typedef struct tjson_value_t tjson_value;

struct tjson_object_t {
    const char **names;
    tjson_value **values;
    size_t count;
};

struct tjson_array_t {
    tjson_value **items;
    size_t count;
};

union tjson_metadata_t {
    const char *string;
    double number;
    int boolean;
    int null;
    tjson_object *object;
    tjson_array *array;
};

struct tjson_value_t {
    tjson_value_type type;
    tjson_metadata value;
};

tjson_value *tjson_parse_file(const char *path);
void tjson_free(tjson_value *value);
// json_object* tjson_parse_file(int fd);

int tjson_isnull(tjson_value *value);
int tjson_isstring(tjson_value *value);
int tjson_isnumber(tjson_value *value);
int tjson_isobject(tjson_value *value);
int tjson_isarray(tjson_value *value);
int tjson_isboolean(tjson_value *value);


#define TJSON_DEBUG() printf("%s:%d\n", __func__, __LINE__)

#endif
