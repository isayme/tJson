#include "tjson.h"

void print_json(tjson_value *value, int indent)
{
    int i, j;
    if (NULL == value) return;
    
    for (i = 0; i < indent; i++) printf(" ");
    
    switch (value->type)
    {
        case TJSON_TYPE_STRING:
            printf("STRING : %s\n", tjson_value_string(value));
            break;
        case TJSON_TYPE_NUMBER:
            printf("NUMBER : %f\n", tjson_value_number(value));
            break;
        case TJSON_TYPE_NULL:
            printf("NULL : null\n");
            break;
        case TJSON_TYPE_BOOLEAN:
            printf("BOOLEAN : %d\n", tjson_value_boolean(value));
            break;
        case TJSON_TYPE_OBJECT:
            printf("OBJECT : \n");
            for (i = 0; i < value->data.object->count; i++)
            {
                for (j = 0; j < indent + 4; j++) printf(" ");
                printf("%s : ", value->data.object->keys[i]);
                print_json(tjson_value_object(value, value->data.object->keys[i]), indent + 4);
            }
            break;
        case TJSON_TYPE_ARRAY:
            printf("ARRAY : \n");
            for (i = 0; i < value->data.array->count; i++)
            {
                print_json(tjson_value_array(value, i), indent + 4);
            }
            break;
        default:
            printf("unkown type: %d\n", value->type);
        break;
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
    
    
    print_json(root, 0);
    
    tjson_value_free(&root);
    tjson_value_free(&root);
    tjson_value_free(&root);

    return 0;
}
