## tJson ##
A tiny JSON parser written in LINUX C.


## APIs ##

```
// parse json from metadata
tjson_value *tjson_parse_data(const char *json_data);

// parse json from file
tjson_value *tjson_parse_file(const char *path);

// free memory of a json object
void tjson_value_free(tjson_value **value);
```

```
// get type of a json object
tjson_valuetype tjson_gettype(const tjson_value *value);

// json object type check
int tjson_isstring(const tjson_value *value);
int tjson_isnumber(const tjson_value *value);
int tjson_isboolean(const tjson_value *value);
int tjson_isnull(const tjson_value *value);
int tjson_isobject(const tjson_value *value);
int tjson_isarray(const tjson_value *value);
int tjson_iserror(const tjson_value *value);
```
```
// return string pointer of a `string` type json object, otherwise return NULL.
const char *tjson_value_string(const tjson_value *value);

// return number value of a `number` type json object, otherwise return 0.
double tjson_value_number(const tjson_value *value);

// return boolean value of a `boolean` type json object, otherwise return -1.
int tjson_value_boolean(const tjson_value *value);

// return 1 if value is `null` type, otherwise return -1.
int tjson_value_null(const tjson_value *value);

// return json object pointer specified by `key`.
tjson_value *tjson_value_object(const tjson_value *value, const char *key);

// return json object pointer specified by `index`.
tjson_value *tjson_value_array(const tjson_value *value, int index);
```
