/*
 Parson ( http://kgabis.github.com/parson/ )
 Copyright (c) 2012 Krzysztof Gabis
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
*/

#include "parson.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#define ERROR                      0
#define SUCCESS                    1
#define STARTING_CAPACITY         15
#define ARRAY_MAX_CAPACITY    122880 /* 15*(2^13) */
#define OBJECT_MAX_CAPACITY      960 /* 15*(2^6)  */
#define MAX_NESTING               19
#define sizeof_token(a)       (sizeof(a) - 1)
#define skip_char(str)        ((*str)++)
#define skip_whitespaces(str) while (isspace(**string)) { skip_char(string); }
#define MAX(a, b)             ((a) > (b) ? (a) : (b))

#define parson_malloc(a)     malloc(a)
#define parson_free(a)       free((void*)a)
#define parson_realloc(a, b) realloc(a, b)

/* Type definitions */
typedef union parson_value_value {
    char        *string; // we may want to change this.
    double       number;
    PARSON_Object *object;
    PARSON_Array  *array;
    int          boolean;
    int          null;
} PARSON_Value_Value;

struct parson_value_t {
    PARSON_Value_Type     type;
    PARSON_Value_Value    value;
};

struct parson_object_t {
    char **names;
    PARSON_Value **values;
    size_t       count;
    size_t       capacity;
};

struct parson_array_t {
    PARSON_Value **items;
    size_t       count;
    size_t       capacity;
};

/* Various */
static int    try_realloc(void **ptr, size_t new_size);
static char * parson_strndup(char *string, size_t n);
static int    is_utf(unsigned char *string);
static int    is_decimal(char *string, size_t length);

/* Various */
static int try_realloc(void **ptr, size_t new_size) {
    void *reallocated_ptr = parson_realloc(*ptr, new_size);
    if (!reallocated_ptr) { return ERROR; }
    *ptr = reallocated_ptr;
    return SUCCESS;
}

static char * parson_strndup(char *string, size_t n) {
    char *output_string = (char*)parson_malloc(n + 1);
    if (!output_string) { return NULL; }
    output_string[n] = '\0';
    strncpy(output_string, string, n);
    return output_string;
}

static int is_utf(unsigned char *s) {
    return isxdigit(s[0]) && isxdigit(s[1]) && isxdigit(s[2]) && isxdigit(s[3]);
}

static int is_decimal(char *string, size_t length) {
    if (length > 1 && string[0] == '0' && string[1] != '.') { return 0; }
    if (length > 2 && !strncmp(string, "-0", 2) && string[2] != '.') { return 0;
}
    while (length--) { if (strchr("xX", string[length])) { return 0; } }
    return 1;
}

/* JSON Object */
PARSON_Object * parson_object_init(void) {
    PARSON_Object *new_obj =
(PARSON_Object*)parson_malloc(sizeof(PARSON_Object));
    if (!new_obj) { return NULL; }
    new_obj->names = (char**)NULL;
    new_obj->values = (PARSON_Value**)NULL;
    new_obj->capacity = 0;
    new_obj->count = 0;
    return new_obj;
}

int parson_object_add(PARSON_Object *object, char *name, PARSON_Value
*value) {
    size_t index;
    if (object->count >= object->capacity) {
        size_t new_capacity = MAX(object->capacity * 2, STARTING_CAPACITY);
        if (new_capacity > OBJECT_MAX_CAPACITY) { return ERROR; }
        if (parson_object_resize(object, new_capacity) == ERROR) { return ERROR;
}
    }
    if (parson_object_get_value(object, name) != NULL) { return ERROR; }
    index = object->count;
    object->names[index] = parson_strndup(name, strlen(name));
    if (!object->names[index]) { return ERROR; }
    object->values[index] = value;
    object->count++;
    return SUCCESS;
}

int parson_object_resize(PARSON_Object *object, size_t capacity) {
    if (try_realloc((void**)&object->names, capacity * sizeof(char*)) == ERROR)
{ return ERROR; }
    if (try_realloc((void**)&object->values, capacity * sizeof(PARSON_Value*))
== ERROR) { return ERROR; }
    object->capacity = capacity;
    return SUCCESS;
}

PARSON_Value * parson_object_nget_value(PARSON_Object *object, char *name, size_t n) {
    size_t i, name_length;
    for (i = 0; i < parson_object_get_count(object); i++) {
        name_length = strlen(object->names[i]);
        if (name_length != n) { continue; }
        if (strncmp(object->names[i], name, n) == 0) { return object->values[i];
}
    }
    return NULL;
}

void parson_object_free(PARSON_Object *object) {
    while(object->count--) {
        parson_free(object->names[object->count]);
        parson_value_free(object->values[object->count]);
    }
    parson_free(object->names);
    parson_free(object->values);
    parson_free(object);
}

/* JSON Array */
PARSON_Array * parson_array_init(void) {
    PARSON_Array *new_array =
(PARSON_Array*)parson_malloc(sizeof(PARSON_Array));
    if (!new_array) { return NULL; }
    new_array->items = (PARSON_Value**)NULL;
    new_array->capacity = 0;
    new_array->count = 0;
    return new_array;
}

int parson_array_add(PARSON_Array *array, PARSON_Value *value) {
    if (array->count >= array->capacity) {
        size_t new_capacity = MAX(array->capacity * 2, STARTING_CAPACITY);
        if (new_capacity > ARRAY_MAX_CAPACITY) { return ERROR; }
        if (!parson_array_resize(array, new_capacity)) { return ERROR; }
    }
    array->items[array->count] = value;
    array->count++;
    return SUCCESS;
}

int parson_array_resize(PARSON_Array *array, size_t capacity) {
    if (try_realloc((void**)&array->items, capacity * sizeof(PARSON_Value*)) ==
ERROR) { return ERROR; }
    array->capacity = capacity;
    return SUCCESS;
}

void parson_array_free(PARSON_Array *array) {
    while (array->count--) { parson_value_free(array->items[array->count]); }
    parson_free(array->items);
    parson_free(array);
}

/* JSON Value */
PARSON_Value * parson_value_init_object(void) {
    PARSON_Value *new_value = (PARSON_Value*)parson_malloc(sizeof(PARSON_Value));
    if (!new_value) { return NULL; }
    new_value->type = JSONObject;
    new_value->value.object = parson_object_init();
    if (!new_value->value.object) { parson_free(new_value); return NULL; }
    return new_value;
}

PARSON_Value * parson_value_init_array(void) {
    PARSON_Value *new_value = (PARSON_Value*)parson_malloc(sizeof(PARSON_Value));
    if (!new_value) { return NULL; }
    new_value->type = JSONArray;
    new_value->value.array = parson_array_init();
    if (!new_value->value.array) { parson_free(new_value); return NULL; }
    return new_value;
}

PARSON_Value * parson_value_init_string(char *string) {
    PARSON_Value *new_value = (PARSON_Value*)parson_malloc(sizeof(PARSON_Value));
    if (!new_value) { return NULL; }
    new_value->type = JSONString;
    new_value->value.string = string;
    return new_value;
}

PARSON_Value * parson_value_init_number(double number) {
    PARSON_Value *new_value = (PARSON_Value*)parson_malloc(sizeof(PARSON_Value));
    if (!new_value) { return NULL; }
    new_value->type = JSONNumber;
    new_value->value.number = number;
    return new_value;
}

PARSON_Value * parson_value_init_boolean(int boolean) {
    PARSON_Value *new_value = (PARSON_Value*)parson_malloc(sizeof(PARSON_Value));
    if (!new_value) { return NULL; }
    new_value->type = JSONBoolean;
    new_value->value.boolean = boolean;
    return new_value;
}

PARSON_Value * parson_value_init_null(void) {
    PARSON_Value *new_value = (PARSON_Value*)parson_malloc(sizeof(PARSON_Value));
    if (!new_value) { return NULL; }
    new_value->type = JSONNull;
    return new_value;
}

/* Parser */
void parson_skip_quotes(char **string) {
    skip_char(string);
    while (**string != '\"') {
        if (**string == '\0') { return; }
        if (**string == '\\') { skip_char(string); if (**string == '\0') {
return; }}
        skip_char(string);
    }
    skip_char(string);
}

/* Returns contents of a string inside double quotes and parses escaped
 characters inside.
 Example: "\u006Corem ipsum" -> lorem ipsum */
char * parson_get_processed_string(char **string) {
    char *string_start = *string;
    char *output, *processed_ptr, *unprocessed_ptr, current_char;
    unsigned int utf_val;
    parson_skip_quotes(string);
    if (**string == '\0') { return NULL; }
    output = parson_strndup(string_start + 1, *string  - string_start - 2);
    if (!output) { return NULL; }
    processed_ptr = unprocessed_ptr = output;
    while (*unprocessed_ptr) {
        current_char = *unprocessed_ptr;
        if (current_char == '\\') {
            unprocessed_ptr++;
            current_char = *unprocessed_ptr;
            switch (current_char) {
                case '\"': case '\\': case '/': break;
                case 'b': current_char = '\b'; break;
                case 'f': current_char = '\f'; break;
                case 'n': current_char = '\n'; break;
                case 'r': current_char = '\r'; break;
                case 't': current_char = '\t'; break;
                case 'u':
                    unprocessed_ptr++;
                    if (!is_utf((unsigned char*)unprocessed_ptr) ||
                        sscanf(unprocessed_ptr, "%4x", &utf_val) == EOF) {
                        parson_free(output); return NULL;
                    }
                    if (utf_val < 0x80) {
                        current_char = utf_val;
                    } else if (utf_val < 0x800) {
                        *processed_ptr++ = (utf_val >> 6) | 0xC0;
                        current_char = ((utf_val | 0x80) & 0xBF);
                    } else {
                        *processed_ptr++ = (utf_val >> 12) | 0xE0;
                        *processed_ptr++ = (((utf_val >> 6) | 0x80) & 0xBF);
                        current_char = ((utf_val | 0x80) & 0xBF);
                    }
                    unprocessed_ptr += 3;
                    break;
                default:
                    parson_free(output);
                    return NULL;
                    break;
            }
        } else if ((unsigned char)current_char < 0x20) { /* 0x00-0x19 are
invalid characters for json string (http://www.ietf.org/rfc/rfc4627.txt) */
            parson_free(output);
            return NULL;
        }
        *processed_ptr = current_char;
        processed_ptr++;
        unprocessed_ptr++;
    }
    *processed_ptr = '\0';
    if (try_realloc((void**)&output, strlen(output) + 1) == ERROR) { return
NULL; }
    return output;
}

PARSON_Value * parse_value(char **string, size_t nesting) {
    if (nesting > MAX_NESTING) {
        printf("JSON: Exceeds MAX_NESTING\n");
        return NULL;
    }
    skip_whitespaces(string);
    switch (**string) {
        case '{':
            printf("JSON: Parsing Object Value\n");
            return parse_object_value(string, nesting + 1);
        case '[':
            printf("Parsing Array Value\n");
            return parse_array_value(string, nesting + 1);
        case '\"': case '\'':
            return parse_string_value(string);
        case 'f': case 't':
            return parse_boolean_value(string);
        case '-':
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            return parse_number_value(string);
        case 'n':
            return parse_null_value(string);
        default:
            printf("JSON: Unknown character: %c",**string);
            return NULL;
    }
}

PARSON_Value * parse_object_value(char **string, size_t nesting) {
    PARSON_Value *output_value = parson_value_init_object(), *new_value = NULL;
    PARSON_Object *output_object = parson_value_get_object(output_value);
    char *new_key = NULL;
    if (!output_value) { return NULL; }
    skip_char(string);
    skip_whitespaces(string);
    if (**string == '}') { skip_char(string); return output_value; } /* empty
object */
    while (**string != '\0') {
        new_key = parson_get_processed_string(string);
        skip_whitespaces(string);
        if (!new_key || **string != ':') {
            parson_value_free(output_value);
            return NULL;
        }
        skip_char(string);
        new_value = parse_value(string, nesting);
        if (!new_value) {
            parson_free(new_key);
            parson_value_free(output_value);
            return NULL;
        }
        if(!parson_object_add(output_object, new_key, new_value)) {
            parson_free(new_key);
            parson_free(new_value);
            parson_value_free(output_value);
            return NULL;
        }
        parson_free(new_key);
        skip_whitespaces(string);
        if (**string != ',') { break; }
        skip_char(string);
        skip_whitespaces(string);
    }
    skip_whitespaces(string);
    if (**string != '}' || /* Trim object after parsing is over */
         parson_object_resize(output_object,
parson_object_get_count(output_object)) == ERROR) {
        parson_value_free(output_value);
        return NULL;
    }
    skip_char(string);
    return output_value;
}

PARSON_Value * parse_array_value(char **string, size_t nesting) {
    PARSON_Value *output_value = parson_value_init_array(), *new_array_value =
NULL;
    PARSON_Array *output_array = parson_value_get_array(output_value);
    if (!output_value) { return NULL; }
    skip_char(string);
    skip_whitespaces(string);
    if (**string == ']') { /* empty array */
        skip_char(string);
        return output_value;
    }
    while (**string != '\0') {
        new_array_value = parse_value(string, nesting);
        if (!new_array_value) {
            parson_value_free(output_value);
            return NULL;
        }
        if(parson_array_add(output_array, new_array_value) == ERROR) {
            parson_free(new_array_value);
            parson_value_free(output_value);
            return NULL;
        }
        skip_whitespaces(string);
        if (**string != ',') { break; }
        skip_char(string);
        skip_whitespaces(string);
    }
    skip_whitespaces(string);
    if (**string != ']' || /* Trim array after parsing is over */
         parson_array_resize(output_array, parson_array_get_count(output_array))
== ERROR) {
        parson_value_free(output_value);
        return NULL;
    }
    skip_char(string);
    return output_value;
}

PARSON_Value * parse_string_value(char **string) {
    char *new_string = parson_get_processed_string(string);
    if (!new_string) { return NULL; }
    return parson_value_init_string(new_string);
}

PARSON_Value * parse_boolean_value(char **string) {
    size_t true_token_size = sizeof_token("true");
    size_t false_token_size = sizeof_token("false");
    if (strncmp("true", *string, true_token_size) == 0) {
        *string += true_token_size;
        return parson_value_init_boolean(1);
    } else if (strncmp("false", *string, false_token_size) == 0) {
        *string += false_token_size;
        return parson_value_init_boolean(0);
    }
    return NULL;
}

PARSON_Value * parse_number_value(char **string) {
    char *end;
    double number = strtod(*string, &end);
    PARSON_Value *output_value;
    if (is_decimal(*string, end - *string)) {
        *string = end;
        output_value = parson_value_init_number(number);
    } else {
        output_value = NULL;
    }
    return output_value;
}

PARSON_Value * parse_null_value(char **string) {
    size_t token_size = sizeof_token("null");
    if (strncmp("null", *string, token_size) == 0) {
        *string += token_size;
        return parson_value_init_null();
    }
    return NULL;
}

/* Parser API */
PARSON_Value * parson_parse_file(char *filename) {
    FILE *fp = fopen(filename, "r");
    size_t file_size;
    char *file_contents;
    PARSON_Value *output_value;
    if (!fp) {
        printf("JSON Not a valid file.\n");
        return NULL;
    }
    struct stat info;
    fstat(fileno(fp), &info);
    int bytes = sizeof(char) * (info.st_size + 1000);
    file_contents = malloc(bytes);
    if (!file_contents) {
        printf("JSON: No file contents!\n");
        fclose(fp);
        return NULL;
    }
    fread(file_contents, sizeof(char),info.st_size, fp);
    fclose(fp);
    file_contents[info.st_size] = '\0';
    output_value = parson_parse_string(file_contents);
    parson_free(file_contents);
    return output_value;
}

PARSON_Value * parson_parse_string(char *string) {
    char * working = strdup(string);
    // Try to find the first instance of { or [ in the file.
    // In case they put spaces, or some other crap in
    // also, utf8 files seem to have two char BOM at the beginning of
    // the file, which throws this function off unless we skip them.
    while (*working != '{' && *working != '[' && *working != '\0') {
        ++working;
    }
    if (!working || (*working != '{' && *working != '[')) {
        printf("JSON: Must start with { or [ but begins with %c is the first char of %s\n",*working,working);
        return NULL;
    }
    return parse_value((char**)&working, 0);
}

/* JSON Object API */
PARSON_Value * parson_object_get_value(PARSON_Object *object, char *name) {
    return parson_object_nget_value(object, name, strlen(name));
}

char * parson_object_get_string(PARSON_Object *object, char *name) {
    return parson_value_get_string(parson_object_get_value(object, name));
}

double parson_object_get_number(PARSON_Object *object, char *name) {
    return parson_value_get_number(parson_object_get_value(object, name));
}

PARSON_Object * parson_object_get_object(PARSON_Object *object, char *name) {
    return parson_value_get_object(parson_object_get_value(object, name));
}

PARSON_Array * parson_object_get_array(PARSON_Object *object, char *name) {
    return parson_value_get_array(parson_object_get_value(object, name));
}

int parson_object_get_boolean(PARSON_Object *object, char *name) {
    return parson_value_get_boolean(parson_object_get_value(object, name));
}

PARSON_Value * parson_object_dotget_value(PARSON_Object *object, char *name) {
    char *dot_position = strchr(name, '.');
    if (!dot_position) { return parson_object_get_value(object, name); }
    object = parson_value_get_object(parson_object_nget_value(object, name,
dot_position - name));
    return parson_object_dotget_value(object, dot_position + 1);
}

char * parson_object_dotget_string(PARSON_Object *object, char *name) {
    return parson_value_get_string(parson_object_dotget_value(object, name));
}

double parson_object_dotget_number(PARSON_Object *object, char *name) {
    return parson_value_get_number(parson_object_dotget_value(object, name));
}

PARSON_Object * parson_object_dotget_object(PARSON_Object *object, char *name) {
    return parson_value_get_object(parson_object_dotget_value(object, name));
}

PARSON_Array * parson_object_dotget_array(PARSON_Object *object, char *name) {
    return parson_value_get_array(parson_object_dotget_value(object, name));
}

int parson_object_dotget_boolean(PARSON_Object *object, char *name)
{
    return parson_value_get_boolean(parson_object_dotget_value(object, name));
}

size_t parson_object_get_count(PARSON_Object *object) {
    return object ? object->count : 0;
}

char * parson_object_get_name(PARSON_Object *object, size_t index) {
    if (index >= parson_object_get_count(object)) { return NULL; }
    return object->names[index];
}

/* JSON Array API */
PARSON_Value * parson_array_get_value(PARSON_Array *array, size_t index) {
    if (index >= parson_array_get_count(array)) { return NULL; }
    return array->items[index];
}

char * parson_array_get_string(PARSON_Array *array, size_t index) {
    return parson_value_get_string(parson_array_get_value(array, index));
}

double parson_array_get_number(PARSON_Array *array, size_t index) {
    return parson_value_get_number(parson_array_get_value(array, index));
}

PARSON_Object * parson_array_get_object(PARSON_Array *array, size_t index)
{
    return parson_value_get_object(parson_array_get_value(array, index));
}

PARSON_Array * parson_array_get_array(PARSON_Array *array, size_t index) {
    return parson_value_get_array(parson_array_get_value(array, index));
}

int parson_array_get_boolean(PARSON_Array *array, size_t index) {
    return parson_value_get_boolean(parson_array_get_value(array, index));
}

size_t parson_array_get_count(PARSON_Array *array) {
    return array ? array->count : 0;
}

/* JSON Value API */
PARSON_Value_Type parson_value_get_type(PARSON_Value *value) {
    return value ? value->type : JSONError;
}

PARSON_Object * parson_value_get_object(PARSON_Value *value) {
    return parson_value_get_type(value) == JSONObject ? value->value.object :
NULL;
}

PARSON_Array * parson_value_get_array(PARSON_Value *value) {
    return parson_value_get_type(value) == JSONArray ? value->value.array :
NULL;
}

char * parson_value_get_string(PARSON_Value *value) {
    return parson_value_get_type(value) == JSONString ? value->value.string :
NULL;
}

double parson_value_get_number(PARSON_Value *value) {
    return parson_value_get_type(value) == JSONNumber ? value->value.number : 0;
}

int parson_value_get_boolean(PARSON_Value *value) {
    return parson_value_get_type(value) == JSONBoolean ? value->value.boolean :
-1;
}

void parson_value_free(PARSON_Value *value) {
    switch (parson_value_get_type(value)) {
        case JSONObject:
            parson_object_free(value->value.object);
            break;
        case JSONString:
            if (value->value.string) { parson_free(value->value.string); }
            break;
        case JSONArray:
            parson_array_free(value->value.array);
            break;
        default:
            break;
    }
    parson_free(value);
}
