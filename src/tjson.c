#include "tjson.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define TJSON_SKIP_WHITESPACES(str) do {\
        while (isspace(**str)) (*str)++;\
    } while (0)

int tjson_isnull(tjson_value *value) { return value ? TJSON_VALUE_TYPE_NULL == value->type : 0; }
int tjson_isstring(tjson_value *value) { return value ? TJSON_VALUE_TYPE_STRING == value->type : 0; }
int tjson_isnumber(tjson_value *value) { return value ? TJSON_VALUE_TYPE_NUMBER == value->type : 0; }
int tjson_isobject(tjson_value *value) { return value ? TJSON_VALUE_TYPE_OBJECT == value->type : 0; }
int tjson_isarray(tjson_value *value) { return value ? TJSON_VALUE_TYPE_ARRAY == value->type : 0; }
int tjson_isboolean(tjson_value *value) { return value ? TJSON_VALUE_TYPE_BOOLEAN == value->type : 0; }

tjson_value *tjson_parse(const char **json_data);

void tjson_free(tjson_value *value)
{
    int i;

    if (NULL == value) return;

    switch (value->type)
    {
        case TJSON_VALUE_TYPE_STRING:
        {
            free((void *)value->value.string);
            break;
        }
        case TJSON_VALUE_TYPE_OBJECT:
        {
            for (i = 0; i < value->value.object->count; i++)
            {
                free((void *)value->value.object->names[i]);
                free((void *)value->value.object->values[i]);
            }
            free((void *)value->value.object->names);
            free((void *)value->value.object->values);
            free((void *)value->value.object);
            break;
        }
        case TJSON_VALUE_TYPE_ARRAY:
        {
            for (i = 0; i < value->value.array->count; i++)
            {
                free((void *)value->value.array->items[i]);
            }
            free((void *)value->value.array);
            break;
        }
        default:
        break;
    }

    free((void *)value);
}

tjson_value *tjson_parse_boolean(const char **json_data)
{
    tjson_value *value = NULL;

    value = malloc(sizeof(tjson_value *));
    if (NULL == value) return NULL;
    value->type = TJSON_VALUE_TYPE_BOOLEAN;

    if (0 == strncmp(*json_data, "true", strlen("true")))
    {
        value->value.boolean = 1;
        *json_data = *json_data + strlen("true");
    }
    else if (0 == strncmp(*json_data, "false", strlen("false")))
    {
        value->value.boolean = 0;
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
    value->type = TJSON_VALUE_TYPE_NUMBER;

    value->value.number = strtod((const char *)(*json_data), (char **)json_data);

    return value;
}

tjson_value *tjson_parse_null(const char **json_data)
{
    tjson_value *value = NULL;

    value = malloc(sizeof(tjson_value *));
    if (NULL == value) return NULL;
    value->type = TJSON_VALUE_TYPE_NULL;

    value->value.null = (0 == strncmp(*json_data, "null", strlen("null")));
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
    value->type = TJSON_VALUE_TYPE_STRING;
    
    value->value.string = malloc(len);
    if (NULL == value->value.string)
    {
        free(value);
        return NULL;
    }
    memset((void *)value->value.string, 0, len);
    strncpy((char *)value->value.string, (const char *)(*json_data + 1), len - 1);

    *json_data = *json_data + len + 1;

    return value;
}

int tjson_skip_char(const char **json_data, char ch)
{
    TJSON_SKIP_WHITESPACES(json_data);

    if (ch != **json_data) return 0;
    
    *json_data = *json_data + 1;

    TJSON_SKIP_WHITESPACES(json_data);

    return 1;
}

tjson_value *tjson_parse_array(const char **json_data)
{
    tjson_value *value = NULL;
    tjson_value *item = NULL; 
    int i;

    value = malloc(sizeof(tjson_value *));
    if (NULL == value) return NULL;
    value->type = TJSON_VALUE_TYPE_ARRAY;
    value->value.array = malloc(sizeof(tjson_array));
    if (NULL == value->value.array)
    {        
        free(value);
        return NULL;
    }
    value->value.array->count = 0;
    value->value.array->items = NULL;

    *json_data = *json_data + 1;
    TJSON_SKIP_WHITESPACES(json_data);

    while (']' != **json_data)
    {
        item = tjson_parse(json_data);
        if (NULL == item)
        {
            tjson_free(value);
            return NULL;
        }

        value->value.array->count++;
        value->value.array->items = realloc(value->value.array->items, sizeof(tjson_value *) * value->value.array->count);
        value->value.array->items[value->value.array->count - 1] = item;

        if (0 == tjson_skip_char(json_data, ',') && ']' != **json_data)
        {
            tjson_free(value);
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
    value->type = TJSON_VALUE_TYPE_OBJECT;
    value->value.object = malloc(sizeof(tjson_object));
    if (NULL == value->value.object)
    {
        TJSON_DEBUG();
        free(value);
        return NULL;
    }
    value->value.object->count = 0;
    value->value.object->names = NULL;
    value->value.object->values = NULL;
    
    *json_data = *json_data + 1;
    TJSON_SKIP_WHITESPACES(json_data);

    while ('}' != **json_data)
    {
        if ('\"' != **json_data)
        {
            tjson_free(value);
            return NULL;
        }

        len = 1;
        while ('\"' != *(*json_data + len))
        {
            if ('\0' == *(*json_data + len)) return NULL;
            if ('\\' == *(*json_data + len)) len++;
            len++;
        }

        value->value.object->count++;
        value->value.object->names = realloc(value->value.object->names, sizeof(const char *) * value->value.object->count);
        value->value.object->values = realloc(value->value.object->values, sizeof(tjson_value *) * value->value.object->count);
        value->value.object->names[value->value.object->count - 1] = malloc(len);
        if (NULL == value->value.object->names[value->value.object->count - 1])
        {
            tjson_free(value);
            return NULL;
        }
        strncpy(value->value.object->names[value->value.object->count - 1], *json_data + 1, len - 1);
        *json_data = *json_data + len + 1;

        if (0 == tjson_skip_char(json_data, ':'))
        {
            tjson_free(value);
            return NULL;
        }

        item = tjson_parse(json_data);
        if (NULL == item)
        {
            tjson_free(value);
            return NULL;
        }

        value->value.object->values[value->value.object->count - 1] = item;
        

        if (0 == tjson_skip_char(json_data, ',') && '}' != **json_data)
        {
            tjson_free(value);
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
        case '-':
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

tjson_value *tjson_parse_file(const char *path)
{
    FILE *fp = NULL;
    tjson_value *root = NULL;
    
    struct stat st;
    char *json_data = NULL;
    const char *tmp;
    if (NULL == path) return NULL;
    
    if (-1 == stat(path, &st)) return NULL;
    json_data = malloc(st.st_size + 1);
    if (NULL == json_data) return NULL;
    memset(json_data, 0, st.st_size + 1);
    
    fp = fopen(path, "rb");
    if (NULL == fp) return NULL;
    
    fread(json_data, 1, st.st_size, fp);

    tmp = json_data;
    root = tjson_parse(&tmp);
    
    free(json_data);
    fclose(fp);
    return root;
}

