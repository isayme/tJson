#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "tjson.h"

#define TJSON_SKIP_WHITESPACES(str) do {\
        while (isspace(**str)) (*str)++;\
    } while (0)

int tjson_skip_char(const char **json_data, char ch)
{
    TJSON_SKIP_WHITESPACES(json_data);

    if (ch != **json_data) return 0;
    
    *json_data = *json_data + 1;

    TJSON_SKIP_WHITESPACES(json_data);

    return 1;
}

tjson_valuetype tjson_gettype(const tjson_value *value) { return value ? value->type : TJSON_TYPE_ERROR; }
int tjson_isstring(const tjson_value *value) { return TJSON_TYPE_STRING == tjson_gettype(value); }
int tjson_isnumber(const tjson_value *value) { return TJSON_TYPE_NUMBER == tjson_gettype(value); }
int tjson_isboolean(const tjson_value *value) { return TJSON_TYPE_BOOLEAN == tjson_gettype(value); }
int tjson_isnull(const tjson_value *value) { return TJSON_TYPE_NULL == tjson_gettype(value); }
int tjson_isobject(const tjson_value *value) { return TJSON_TYPE_OBJECT == tjson_gettype(value); }
int tjson_isarray(const tjson_value *value) { return TJSON_TYPE_ARRAY == tjson_gettype(value); }
int tjson_iserror(const tjson_value *value) { return TJSON_TYPE_ERROR == tjson_gettype(value); }

void tjson_value_free(tjson_value *value)
{
    int i;
    if (NULL == value) return;

    switch (value->type)
    {
        case TJSON_TYPE_STRING:
        {
            free((void *)value->data.string);
            break;
        }
        case TJSON_TYPE_OBJECT:
        {
            for (i = 0; i < value->data.object->count; i++)
            {
                free((void *)value->data.object->keys[i]);
                tjson_value_free(value->data.object->values[i]);
            }
            free(value->data.object->keys);
            free(value->data.object->values);
            free(value->data.object);
            break;
        }
        case TJSON_TYPE_ARRAY:
        {
            for (i = 0; i < value->data.array->count; i++)
            {
                tjson_value_free(value->data.array->items[i]);
            }
            free(value->data.array);
            break;
        }
        default:
        break;
    }

    free(value);
}

tjson_value *tjson_parse(const char **json_data);

tjson_value *tjson_parse_boolean(const char **json_data)
{
    tjson_value *value = NULL;

    value = malloc(sizeof(tjson_value *));
    if (NULL == value) return NULL;
    value->type = TJSON_TYPE_BOOLEAN;

    if (0 == strncmp(*json_data, "true", strlen("true")))
    {
        value->data.boolean = 1;
        *json_data = *json_data + strlen("true");
    }
    else if (0 == strncmp(*json_data, "false", strlen("false")))
    {
        value->data.boolean = 0;
        *json_data = *json_data + strlen("false");
    }
    else
    {
        free(value);
        return NULL;
    }

    return value;
}

tjson_value *tjson_parse_number(const char **json_data)
{
    tjson_value *value = NULL;

    value = malloc(sizeof(tjson_value *));
    if (NULL == value) return NULL;
    value->type = TJSON_TYPE_NUMBER;

    value->data.number = strtod((const char *)(*json_data), (char **)json_data);

    return value;
}

tjson_value *tjson_parse_null(const char **json_data)
{
    tjson_value *value = NULL;

    value = malloc(sizeof(tjson_value *));
    if (NULL == value) return NULL;
    value->type = TJSON_TYPE_NULL;

    value->data.null = (0 == strncmp(*json_data, "null", strlen("null")));
    *json_data = *json_data + strlen("null");

    return value;
}

tjson_value *tjson_parse_string(const char **json_data)
{
    tjson_value *value = NULL;
    int len = 1;
    
    while ('\"' != *(*json_data + len))
    {
        if ('\0' == *(*json_data + len)) return NULL;
        if ('\\' == *(*json_data + len)) len++;
        len++;
    }
    
    value = malloc(sizeof(tjson_value *));
    if (NULL == value) return NULL;
    value->type = TJSON_TYPE_STRING;
    
    value->data.string = malloc(len);
    if (NULL == value->data.string)
    {
        free(value);
        return NULL;
    }
    memset((void *)value->data.string, 0, len);
    strncpy((char *)value->data.string, (const char *)(*json_data + 1), len - 1);

    *json_data = *json_data + len + 1;

    return value;
}

tjson_value *tjson_parse_array(const char **json_data)
{
    tjson_value *value = NULL;
    tjson_value *item = NULL; 
    int i;

    value = malloc(sizeof(tjson_value *));
    if (NULL == value) return NULL;
    value->type = TJSON_TYPE_ARRAY;
    value->data.array = malloc(sizeof(tjson_array));
    if (NULL == value->data.array)
    {        
        free(value);
        return NULL;
    }
    value->data.array->count = 0;
    value->data.array->items = NULL;

    *json_data = *json_data + 1;

    while (']' != **json_data)
    {
        item = tjson_parse(json_data);
        if (NULL == item)
        {
            tjson_value_free(value);
            return NULL;
        }

        value->data.array->count++;
        value->data.array->items = realloc(value->data.array->items, sizeof(tjson_value *) * value->data.array->count);
        value->data.array->items[value->data.array->count - 1] = item;

        if (0 == tjson_skip_char(json_data, ',') && ']' != **json_data)
        {
            tjson_value_free(value);
            return NULL;
        }
    }

    *json_data = *json_data + 1;

    return value;
}

tjson_value *tjson_parse_object(const char **json_data)
{
    tjson_value *value = NULL;
    tjson_value *item = NULL; 
    int len = 1;
    
    value = malloc(sizeof(tjson_value *));
    if (NULL == value) return NULL;
    value->type = TJSON_TYPE_OBJECT;
    value->data.object = malloc(sizeof(tjson_object));
    if (NULL == value->data.object)
    {
        free(value);
        return NULL;
    }
    value->data.object->count = 0;
    value->data.object->keys = NULL;
    value->data.object->values = NULL;
    
    *json_data = *json_data + 1;
    TJSON_SKIP_WHITESPACES(json_data);

    while ('}' != **json_data)
    {
        if ('\"' != **json_data)
        {
            tjson_value_free(value);
            return NULL;
        }

        len = 1;
        while ('\"' != *(*json_data + len))
        {
            if ('\0' == *(*json_data + len)) return NULL;
            if ('\\' == *(*json_data + len)) len++;
            len++;
        }

        value->data.object->count++;
        value->data.object->keys = realloc(value->data.object->keys, sizeof(const char *) * value->data.object->count);
        value->data.object->values = realloc(value->data.object->values, sizeof(tjson_value *) * value->data.object->count);
        value->data.object->keys[value->data.object->count - 1] = malloc(len);
        if (NULL == value->data.object->keys[value->data.object->count - 1])
        {
            tjson_value_free(value);
            return NULL;
        }
        memset((void *)value->data.object->keys[value->data.object->count - 1], 0, len);
        strncpy((char *)value->data.object->keys[value->data.object->count - 1], *json_data + 1, len - 1);

        *json_data = *json_data + len + 1;

        if (0 == tjson_skip_char(json_data, ':'))
        {
            tjson_value_free(value);
            return NULL;
        }

        item = tjson_parse(json_data);
        if (NULL == item)
        {
            tjson_value_free(value);
            return NULL;
        }

        value->data.object->values[value->data.object->count - 1] = item;
        

        if (0 == tjson_skip_char(json_data, ',') && '}' != **json_data)
        {
            tjson_value_free(value);
            return NULL;
        }
    }
    
    *json_data = *json_data + 1;
    return value;
}

tjson_value *tjson_parse(const char **json_data)
{    
    tjson_value *value = NULL;
    
    TJSON_SKIP_WHITESPACES(json_data);
    
    switch (**json_data) {
        case '[':   // array
        {    
            value = tjson_parse_array(json_data);
            break;
        }
        case '{':   // object
        {    
            value = tjson_parse_object(json_data);
            break;
        }
        case '\"':  // string
        {    
            value = tjson_parse_string(json_data);
            break;
        }
        case 'f':   // boolean
        case 't':
        {    
            value = tjson_parse_boolean(json_data);
            break;
        }
        case 'n':   // null
        {    
            value = tjson_parse_null(json_data);
            break;
        }
        case '-':   // number
        case '.':   // definition @http://json.org/ donot support number begin with POINT(.)
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        {    
            value = tjson_parse_number(json_data);
            break;
        }
        default:
        break;
    }
    
    return value;
}

tjson_value *tjson_parse_data(const char *json_data)
{
    return tjson_parse(&json_data);
}
tjson_value *tjson_parse_file(const char *path)
{
    FILE *fp = NULL;
    tjson_value *root = NULL;
    
    struct stat st;
    char *json_data = NULL;

    if (NULL == path) return NULL;
    
    if (-1 == stat(path, &st)) return NULL;
    json_data = malloc(st.st_size + 1);
    if (NULL == json_data) return NULL;
    memset(json_data, 0, st.st_size + 1);
    
    fp = fopen(path, "rb");
    if (NULL == fp) return NULL;
    
    fread(json_data, 1, st.st_size, fp);

    root = tjson_parse_data(json_data);
    
    free(json_data);
    fclose(fp);
    return root;
}

const char *tjson_value_string(const tjson_value *value)
{ 
    return (!value || !tjson_isstring(value)) ? NULL : value->data.string;
}
double tjson_value_number(const tjson_value *value)
{ 
    return (!value || !tjson_isnumber(value)) ? 0 : value->data.number;
}
int tjson_value_boolean(const tjson_value *value)
{ 
    return (!value || !tjson_isboolean(value)) ? -1 : value->data.boolean;
}
int tjson_value_null(const tjson_value *value)
{ 
    return (!value || !tjson_isnull(value)) ? -1 : value->data.null;
}
tjson_value *tjson_value_object(const tjson_value *value, const char *key)
{
    int i;

    if (NULL == value || NULL == key || !tjson_isobject(value)) return NULL;

    for (i = 0; i < value->data.object->count; i++)
    {
        if (0 == strcmp(key, value->data.object->keys[i]))
        {
            return value->data.object->values[i];
        }
    }

    return NULL;
}
tjson_value *tjson_value_array(const tjson_value *value, int index)
{
    if (NULL == value
        || !tjson_isarray(value)
        || value->data.array->count <= index) return NULL;

    return value->data.array->items[index];
}
