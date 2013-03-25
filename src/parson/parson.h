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

#ifndef parson_parson_h
#define parson_parson_h

#ifdef __cplusplus
extern "C"
{
#endif    
    
#include <stddef.h>   /* size_t */    
    
/* Types and enums */
typedef struct parson_object_t PARSON_Object;
typedef struct parson_array_t  PARSON_Array;
typedef struct parson_value_t  PARSON_Value;

typedef enum parson_value_type {
    JSONError   = 0,
    JSONNull    = 1,
    JSONString  = 2,
    JSONNumber  = 3,
    JSONObject  = 4,
    JSONArray   = 5,
    JSONBoolean = 6
} PARSON_Value_Type;

/* Parser */
/* Parses first JSON value in a file, returns NULL in case of error */
PARSON_Value  * parson_parse_file(char *filename);

/*  Parses first JSON value in a string, returns NULL in case of error */
PARSON_Value  *     parson_parse_string(char *string);
void                parson_skip_quotes(char **string);
char *              parson_get_processed_string(char **string);
PARSON_Value *      parse_object_value(char **string, size_t nesting);
PARSON_Value *      parse_array_value(char **string, size_t nesting);
PARSON_Value *      parse_string_value(char **string);
PARSON_Value *      parse_boolean_value(char **string);
PARSON_Value *      parse_number_value(char **string);
PARSON_Value *      parse_null_value(char **string);
PARSON_Value *      parse_value(char **string, size_t nesting);

/* JSON Object */
PARSON_Object *     parson_object_init(void);
int                 parson_object_add(PARSON_Object *object, char *name, PARSON_Value *value);
int                 parson_object_resize(PARSON_Object *object, size_t capacity);
PARSON_Value  *     parson_object_nget_value(PARSON_Object *object, char *name, size_t n);
void                parson_object_free(PARSON_Object *object);
PARSON_Value  *     parson_object_get_value  (PARSON_Object *object, char *name);
char *              parson_object_get_string (PARSON_Object *object, char *name);
PARSON_Object *     parson_object_get_object (PARSON_Object *object, char *name);
PARSON_Array  *     parson_object_get_array  (PARSON_Object *object, char *name);
double              parson_object_get_number (PARSON_Object *object, char *name);
int                 parson_object_get_boolean(PARSON_Object *object, char *name);

/* dotget functions enable addressing values with dot notation in nested
objects,
 just like in structs or c++/java/c# objects (e.g. objectA.objectB.value).
 Because valid names in JSON can contain dots, some values may be inaccessible
 this way. */
PARSON_Value   *     parson_object_dotget_value  (PARSON_Object *object, char *name);
      char     *     parson_object_dotget_string (PARSON_Object *object, char *name);
PARSON_Object  *     parson_object_dotget_object (PARSON_Object *object, char *name);
PARSON_Array   *     parson_object_dotget_array  (PARSON_Object *object, char *name);
double               parson_object_dotget_number (PARSON_Object *object, char *name);
int                  parson_object_dotget_boolean(PARSON_Object *object, char *name);

/* Functions to get available names */
size_t             parson_object_get_count(PARSON_Object *object);
char *    parson_object_get_name (PARSON_Object *object, size_t index);
    
/* JSON Array */
PARSON_Array  *    parson_array_init(void);
int                parson_array_add(PARSON_Array *array, PARSON_Value *value);
int                parson_array_resize(PARSON_Array *array, size_t capacity);
void               parson_array_free(PARSON_Array *array);
PARSON_Value  *    parson_array_get_value  (PARSON_Array *array, size_t index);
char *    parson_array_get_string (PARSON_Array *array, size_t index);
PARSON_Object *    parson_array_get_object (PARSON_Array *array, size_t index);
PARSON_Array  *    parson_array_get_array  (PARSON_Array *array, size_t index);
double             parson_array_get_number (PARSON_Array *array, size_t index);
int                parson_array_get_boolean(PARSON_Array *array, size_t index);
size_t             parson_array_get_count  (PARSON_Array *array);

/* JSON Value */
PARSON_Value *    parson_value_init_object(void);
PARSON_Value *    parson_value_init_array(void);
PARSON_Value *    parson_value_init_string(char *string);
PARSON_Value *    parson_value_init_number(double number);
PARSON_Value *    parson_value_init_boolean(int boolean);
PARSON_Value *    parson_value_init_null(void);
PARSON_Value_Type parson_value_get_type   (PARSON_Value *value);
PARSON_Object *   parson_value_get_object (PARSON_Value *value);
PARSON_Array  *   parson_value_get_array  (PARSON_Value *value);
char *            parson_value_get_string (PARSON_Value *value);
double            parson_value_get_number (PARSON_Value *value);
int               parson_value_get_boolean(PARSON_Value *value);
void              parson_value_free       (PARSON_Value *value);
    
#ifdef __cplusplus
}
#endif

#endif
