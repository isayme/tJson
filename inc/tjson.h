#ifndef TJSON_H
#define TJSON_H

#ifdef __cplusplus
extern "C"
{
#endif

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
    const char **keys;
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

tjson_value *tjson_parse_data(const char *json_data);
tjson_value *tjson_parse_file(const char *path);
void tjson_value_free(tjson_value **root);

tjson_valuetype tjson_gettype(const tjson_value *value);
int tjson_isstring(const tjson_value *value);
int tjson_isnumber(const tjson_value *value);
int tjson_isboolean(const tjson_value *value);
int tjson_isnull(const tjson_value *value);
int tjson_isobject(const tjson_value *value);
int tjson_isarray(const tjson_value *value);
int tjson_iserror(const tjson_value *value);

const char *tjson_value_string(const tjson_value *value);
double tjson_value_number(const tjson_value *value);
int tjson_value_boolean(const tjson_value *value);
int tjson_value_null(const tjson_value *value);
tjson_value *tjson_value_object(const tjson_value *value, const char *key);
tjson_value *tjson_value_array(const tjson_value *value, int index);

#ifdef __cplusplus
}
#endif

#endif
