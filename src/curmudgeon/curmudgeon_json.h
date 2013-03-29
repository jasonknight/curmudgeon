#ifndef CURMUDGEON_JSON_H
#define CURMUDGEON_JSON_H
typedef struct curmudgeon_json cur_json_t;

char *               cur_json_string_value(cur_json_t *);
char *               cur_jsons(cur_json_t * obj,char * key);
char *               cur_json_dumps(cur_json_t * obj);
int                  cur_jsons_set(cur_json_t * obj, char * key, char * value);
int                  cur_json_is_string(cur_json_t *);
int                  cur_json_array_append(cur_json_t * arr, cur_json_t * val);
int                  cur_json_object_set(cur_json_t * obj, char * key, cur_json_t * val);
cur_json_t *         cur_json_object_get(cur_json_t * obj, char * key);
cur_json_t *         cur_json_array();
cur_json_t *         cur_json_string(char * str);
cur_json_t *         cur_json_object();
cur_json_t *         cur_json_from_file(char * filename);
cur_json_t *         cur_json_from_string(char * json_string);
cur_json_t *         _json_decode(char *str);
cur_json_t *         _json_decode_file(char *str);
                // Will take a key like: l1.l2.l3 and return l3
cur_json_t *         _json_drill_down(cur_json_t *obj, char *key);
void                 _json_create_key_chain(cur_json_t *opts, char *key, char type);

#endif
