#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "curmudgeon_json.h"
#include "parson.h"
typedef union parson_value_value {
    char        *string; // we may want to change this.
    double       number;
    PARSON_Object *object;
    PARSON_Array  *array;
    int          boolean;
    int          null;
} PARSON_Value_Value;
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
struct parson_value_t {
    PARSON_Value_Type     type;
    PARSON_Value_Value    value;
};
struct curmudgeon_json {
   PARSON_Value * internal_value; 
};

char * cur_json_string_value(cur_json_t * obj) {
    if (cur_json_is_string(obj)) {
        return parson_value_get_string(obj->internal_value);   
    }
    return NULL;
}
int cur_json_is_array(cur_json_t * obj) {
    if (obj->internal_value->type == JSONArray) {
        return 1;
    }
    return 0;
}
int cur_json_is_string(cur_json_t * obj) {
    if (obj->internal_value->type == JSONString) {
        return 1;
    }
    return 0;
}
int cur_json_array_append(cur_json_t * arr, cur_json_t * val) {
    if (cur_json_is_array(arr)) {
        PARSON_Array * target = parson_value_get_array(arr->internal_value);
        PARSON_Value * new_value = val->internal_value;
        int ret = parson_array_add(target,new_value); 
        return 0;
    } else {
        return -1;
    }
}
cur_json_t * cur_json_object_get(cur_json_t * obj, char * key) {
    PARSON_Object * target = parson_value_get_object(obj->internal_value);
    PARSON_Value * existing_value = parson_object_dotget_value(target,key);
    if ( ! existing_value ) {
        printf("No Value Found\n");
        return NULL;
    }
    cur_json_t * ret = malloc(sizeof(cur_json_t));
    ret->internal_value = existing_value;
    return ret;

}
int cur_json_object_set(cur_json_t * obj, char * key, cur_json_t * val) {
    PARSON_Object * target = parson_value_get_object(obj->internal_value);
    PARSON_Value * existing_value = parson_object_get_value(target,key);
    if (existing_value) {
      if (existing_value->type == JSONString) {
        existing_value->value.string = cur_json_string_value(val);
      }
    } else {
        int ret = parson_object_add(target,key,val->internal_value); 
        if (ret == 1) {
            return 0;
        } else {
            return -1;
        }
    }
    return 0;
}
// just a little helper function
char * cur_jsons(cur_json_t * obj,char * key) {
    PARSON_Object * target = parson_value_get_object(obj->internal_value);
    PARSON_Value * existing_value = parson_object_dotget_value(target,key);
    if (existing_value) {
        return existing_value->value.string;
    }
    return NULL;
}
int cur_jsons_set(cur_json_t * obj, char * key, char * value) {
    char * working_key = key;
    PARSON_Object * target = parson_value_get_object(obj->internal_value);
    PARSON_Value * existing_value = parson_object_dotget_value(target,working_key);
    if ( ! existing_value ) {
        // we'll have to create it
        char * token;
        char * next_token;
        char * free_cpy = strdup(key); // we avoid segfaults this way
        char * cpy = free_cpy;
        while (cpy) {
            next_token = strchr(cpy,'.');
            token = strsep(&cpy,".");
            if (next_token) {
                // we aren't at the end
                // Maybe this part of the key does exist?
                existing_value = parson_object_dotget_value(target,token);
                if (existing_value) {
                    // we don't need to create it
                    target = parson_value_get_object(existing_value);
                    continue;
                }
                PARSON_Value * new_object = parson_value_init_object();
                parson_object_add(target,token, new_object);
                target = parson_value_get_object(new_object);
            } else {
                // we are at the end
                PARSON_Value * new_string_value = parson_value_init_string(value);
                parson_object_add(target,token,new_string_value);
            }
        }
        free(free_cpy); // free our copy
    } else {
       if (existing_value->type == JSONString) {
        existing_value->value.string = value;
      }
    }
    return 0;
}
char * _json_value_to_s(PARSON_Value * value,int depth) {
   char * fmt;
   int i;
   int bytes_to_malloc;
   char * child_text = malloc(sizeof(char) * 1);
   char * new_child_text;
   char * tmp;
   child_text[0] = '\0';
   if (value->type == JSONObject) {
        fmt = "{ %s }";
        char * tfmt;
        PARSON_Object * obj = value->value.object;
        for (i = 0; i < obj->count; i++) {
            if (i == obj->count - 1) {
                tfmt = "%s \"%s\": %s";
            } else {
                tfmt = "%s \"%s\": %s,";
            }
            tmp = _json_value_to_s(obj->values[i],++depth);
            bytes_to_malloc = sizeof(char) * ( strlen(child_text) + strlen(tmp) + strlen(obj->names[i]) + strlen(tfmt) + 1 );
            new_child_text = malloc(bytes_to_malloc);
            sprintf(new_child_text,tfmt,child_text,obj->names[i],tmp);
            free(child_text);
            free(tmp);
            child_text = new_child_text;
        }
        bytes_to_malloc = sizeof(char) * ( strlen(fmt) + strlen(child_text) + 5);
        new_child_text = malloc(bytes_to_malloc);
        sprintf(new_child_text,fmt,child_text);
        free(child_text);
        return new_child_text;
   } else if (value->type == JSONString) {
        fmt = "\"%s\"";
        bytes_to_malloc = sizeof(char) * ( strlen(fmt) + strlen(value->value.string) + strlen(fmt) ); 
        new_child_text = malloc(bytes_to_malloc);
        sprintf(new_child_text,fmt,value->value.string);
        return new_child_text;
   } else if (value->type == JSONArray) {
        fmt = "[ %s ]";
        char * tfmt;
        PARSON_Array * obj = value->value.array;
        for (i = 0; i < obj->count; i++) {
            if (i == obj->count - 1) {
                tfmt = "%s %s";
            } else {
                tfmt = "%s %s,";
            }
            tmp = _json_value_to_s(obj->items[i],++depth);
            bytes_to_malloc = sizeof(char) * ( strlen(child_text) + strlen(tmp) + strlen(tfmt) + 1 );
            new_child_text = malloc(bytes_to_malloc);
            sprintf(new_child_text, tfmt, child_text, tmp);
            free(child_text);
            free(tmp);
            child_text = new_child_text;
        }
        bytes_to_malloc = sizeof(char) * ( strlen(fmt) + strlen(child_text) + 5);
        new_child_text = malloc(bytes_to_malloc);
        sprintf(new_child_text,fmt,child_text);
        free(child_text);
        return new_child_text;

   }
   return NULL;
}
char * cur_json_dumps(cur_json_t * obj) {
    return _json_value_to_s(obj->internal_value,0);
}
cur_json_t * cur_json_array() { 
    cur_json_t * obj = malloc(sizeof(cur_json_t));
    obj->internal_value = parson_value_init_array();
    return obj;
}

cur_json_t * cur_json_string(char * str) { 
    cur_json_t * obj = malloc(sizeof(cur_json_t));
    obj->internal_value = parson_value_init_string(str);
    return obj;
}

cur_json_t * cur_json_object() {
    cur_json_t * obj = malloc(sizeof(cur_json_t));
    obj->internal_value = parson_value_init_object();
    return obj;
}

cur_json_t * cur_json_from_file(char * filename) {
    return _json_decode_file(filename);
}
cur_json_t * _json_decode(char *str) {
    PARSON_Value * root = parson_parse_string(str);
    if (! root ) {
        printf("JSON Error: Failed to parse String\n");
    } else {
        printf("JSON Success.\n");
    }
    cur_json_t * obj = malloc(sizeof(cur_json_t));
    obj->internal_value = root;
    return obj;
}
cur_json_t * _json_decode_file(char *path) {
    PARSON_Value * root = parson_parse_file(path);
    if (! root ) {
        printf("JSON Error: Failed to parse file\n");
    }
    cur_json_t * obj = malloc(sizeof(cur_json_t));
    obj->internal_value = root;
    return obj;
}

