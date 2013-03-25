#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include "curmudgeon_json.h"
void Puti(int);
void Putc(char);
void Puts(char*);
int main(void) {
    setlocale(LC_ALL,"en_US.utf-8");
    cur_json_t * obj = cur_json_object();
    cur_json_t * val = cur_json_string("testing");
    cur_json_object_set(obj,"test",val); 
    printf("Value is: %s\n", cur_json_string_value(val));
    printf("Value is: %s\n", cur_json_string_value( cur_json_object_get(obj,"test") ) );

    obj = cur_json_from_file("options.json");
    printf("testing.inside.hello is: %s\n", cur_json_string_value( cur_json_object_get(obj,"testing.inside.hello") ) );
    printf("with helper: testing.inside.hello is: %s\n", cur_jsons(obj,"testing.inside.hello") );
    cur_jsons_set(obj,"testing.inside.hello","Goodbye Cruel World!");
    cur_jsons_set(obj,"testing.inside.yoyo","Another str value!");
    printf("after jsons_set: testing.inside.hello is: %s\n", cur_jsons(obj,"testing.inside.hello") );
    cur_jsons_set(obj,"something.that.does.not.exist","I will be there soon!");
    printf("after jsons_set: something.that.does.not.exist: %s\n", cur_jsons(obj,"something.that.does.not.exist") );

    cur_json_dumps(obj);
    return 0;
}
void Puti(int i) {
    printf("%d\n",i);
}
void Putc(char c) {
    printf("%c(%x)\n",c,c);
}
void Puts(char * s) {
    printf("%s\n",s);
}

