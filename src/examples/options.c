#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include "curmudgeon.h"
void Puti(int);
void Putc(char);
void Puts(char*);
int main(void) {
    setlocale(LC_ALL,"en_US.utf-8");
    curmudgeon_t *      app = NULL;
    event_t *           event = NULL;
    char *              test = "/hello_world/arg1/arg2";
    if ( cur_init(&app,1) == CUR_OK) {
        cur_opts_t * opts = cur_create_options("\"test\": { \"inside\": \"Hello World\"}"); 
        char * value;
        cur_options(opts,"test.inside",&value);
        printf("value is: %s\n",value);
        if (cur_options_set(opts,"test.inside","Goodbye Cruel World!") == CUR_OK) { 
            cur_options(opts,"test.inside",&value);
            printf("value is: %s\n",value);
        } else {
            printf("something went wrong\n");
        }
        cur_opts_t * nopts = cur_create_options("");
        cur_options_set(nopts,"table.name","my_db_table");
        printf("Dumping: %s\n",json_dumps(nopts->json,0));
        cur_options_set(nopts,"table.encoding","utf8");

        printf("Dumping: %s\n",json_dumps(nopts->json,0));
        cur_options(nopts,"table.name",&value);
        printf("val is: %s\n",value);
        cur_options(nopts,"table.encoding",&value);
        printf("val is: %s\n",value);
        cur_done(&app);
    } else {
        Puts("Could not init");
    }
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

