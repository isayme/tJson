#ifndef TJSON_H
#define TJSON_H

#include <stdio.h>

typedef enum tjson_valuetype {
    TJSON_TYPE_ERROR = 0,
    TJSON_TYPE_STRING,
    TJSON_TYPE_NUMBER,
    TJSON_TYPE_BOOLEAN,
    TJSON_TYPE_NULL,
    TJSON_TYPE_OBJECT,
    TJSON_TYPE_ARRAY
} tjson_valuetype;

typedef struct tjson_object tjson_object;
typedef struct tjson_array tjson_array;

typedef union tjson_metadata tjson_metadata;

typedef struct tjson_value tjson_value;

struct tjson_object {
    const char **names;
    tjson_value **values;
    size_t count;
};

struct tjson_array {
    tjson_value **items;
    size_t count;
};

union tjson_metadata {
    const char *string;
    double number;
    int boolean;
    int null;
    tjson_object *object;
    tjson_array *array;
};

struct tjson_value {
    tjson_valuetype type;
    tjson_metadata data;
};

tjson_value *tjson_parse_file(const char *path);
void tjson_free(tjson_value *value);

int tjson_isstring(tjson_value *value);
int tjson_isnumber(tjson_value *value);
int tjson_isboolean(tjson_value *value);
int tjson_isnull(tjson_value *value);
int tjson_isobject(tjson_value *value);
int tjson_isarray(tjson_value *value);

#define TJSON_DEBUG() printf("%s:%d\n", __func__, __LINE__)

#endif
