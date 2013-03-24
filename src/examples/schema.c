#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "curmudgeon.h"
void Puti(int);
void Putc(char);
void Puts(char*);
int main(void) {
    curmudgeon_t *      app = NULL;
    event_t *           event = NULL;
    char *              test = "/hello_world/arg1/arg2";
    if ( cur_init(&app,1) == CUR_OK) {
        app->schema_version_file = "schema.version"; 
        adapter_t * a = NULL;
        db_mysql(app,&a,"localhost","root","root");
        db_connect( a );
        db_select_db( a ,"curmudgeon");
        printf("Schema version is: %d\n",_get_num_from_file(app->schema_version_file));
        _set_num_in_file(app->schema_version_file,1); 
        printf("Schema version is: %d\n",_get_num_from_file(app->schema_version_file));
        if ( schema_database(a,"test_table",NULL,NULL) == CUR_OK ) {
            printf("migrated\n");
        } else {
            printf("did not migrate\n");
        }
        _set_num_in_file(app->schema_version_file,0); 
        if ( schema_database( a ,"test_table",NULL,NULL) == CUR_OK ) {
            printf("migrated\n");
        } else {
            printf("did not migrate\n");
        }
        schema_database( a ,"test_table","latin1","latin1_general_ci");
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

