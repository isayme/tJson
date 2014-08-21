#include "tjson.h"

void print_object(tjson_value *root)
{
    int i;
    tjson_value *value = NULL;

    if (tjson_isobject(root))
    {
        printf("len = %d\n", root->value.object->count);

        for (i = 0; i < root->value.object->count; i++)
        {
            printf("OBJECT : %s\n", root->value.object->names[i]);

        
        }
    }
}

void print_array(tjson_value *root)
{
    int i;
    tjson_value *value = NULL;

    if (tjson_isarray(root))
    {
        printf("len = %d\n", root->value.array->count);

        for (i = 0; i < root->value.array->count; i++)
        {
            value = root->value.array->items[i];

            switch (value->type)
            {
                case TJSON_VALUE_TYPE_STRING:
                    printf("STRING : %s\n", value->value.string);
                    break;
                case TJSON_VALUE_TYPE_NUMBER:
                    printf("NUMBER : %f\n", value->value.number);
                    break;
                case TJSON_VALUE_TYPE_NULL:
                    printf("NULL : %d\n", value->value.null);
                    break;
                case TJSON_VALUE_TYPE_BOOLEAN:
                    printf("BOOLEAN : %d\n", value->value.boolean);
                    break;
                case TJSON_VALUE_TYPE_ARRAY:
                    print_array(value);
                    break;
                case TJSON_VALUE_TYPE_OBJECT:
                    print_object(value);
                    break;
                default:
                    printf("unkown type: %d\n", value->type);
                break;
            }
        }
    }
    
}

int main(int argc, char **argv)
{
    tjson_value *root = NULL;

    if (argc < 2) 
    {
        printf("must specify a json file!\n");
        return -1;
    }
    
    root = tjson_parse_file(argv[1]);
    if (NULL == root)
    {
        printf("root is NULL\n");
        return -1;
    }
    
    print_array(root);
    
    tjson_free(root);

    return 0;
}
