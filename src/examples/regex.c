#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "curmudgeon.h"
void Puti(int);
void Putc(char);
void Puts(char*);
int hello_world(event_t * e) {
    printf("Hello from the callback first arg is:  %s \n", e->args[0]); 
    return CUR_OK;
}
int main(void) {
    curmudgeon_t *      app = NULL;
    event_t *           event = NULL;
    char *              test = "/hello_world/arg1/arg2";
    if ( cur_init(&app,1) == CUR_OK) {
        cur_register_event(&app,"hello_world.*", 0,hello_world);
        cur_parse_request(test,&event);
        cur_call_handler(app,event);
        regex_t * re = cur_regex("/^\\/hello_(?P<wor>world)(?P<another>\\/something)\\/(?P<else>.*)/i");
        int rc = cur_match(&re,"/hello_world/something/aboutelse",0);
        if ( rc == CUR_OK ) {
            int i;
            for (i = 0; i <re->namec * 2; i++) {
                printf("Named: %s\n",re->named_captures[i]);
            }
            for (i = 0; i <= re->captc; i++) {
                printf("Nummed: %d -> %s\n",i,re->nummed_captures[i]);
            }
        } else {
            printf("it did not match %d\n",rc);
        }
        printf("named search: %s\n",re->named(re,"else"));
        printf("nummed search: %s\n",re->capt(re,3));
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
