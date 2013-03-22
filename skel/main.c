#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curmudgeon.h>
#include <main.h>
void Puti(int);
void Putc(char);
void Puts(char*);

int main(void) {
    #include "conf/app.c"
    #include "conf/database.c"
    #include "conf/schema.c"
    #include "handlers/register_handlers.c"
    return 0;
}



/* stock debug functions */
void Puti(int i) {
    printf("%d\n",i);
}
void Putc(char c) {
    printf("%c(%x)\n",c,c);
}
void Puts(char * s) {
    printf("%s\n",s);
}

