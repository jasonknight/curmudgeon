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
        cur_register_event(&app,"hello_world", 0,hello_world);
        cur_parse_request(test,&event);

        cur_mysql(&app,"localhost","root","root");
        cur_connect( &app );
        cur_exec(&app,"CREATE DATABASE IF NOT EXISTS curmudgeon");
        cur_select_db(&app,"curmudgeon");
        cur_exec(&app,"CREATE TABLE IF NOT EXISTS people (first_name VARCHAR(55), last_name VARCHAR(55))");
        //cur_exec(&app,"INSERT INTO people (first_name, last_name) VALUES ('jason', 'martin')");
        
        // Let's try some queries, they get smaller and easier as we go along.
        // If you just need general behavior, the last ones are when you will use
        // if you need more control, you can use the more raw apis.
        cur_query(&app,"SELECT * FROM people");
        db_row_t * row = NULL;
        while ( cur_next(app,&row) == CUR_OK ) {
            int ri;
            for (ri = 0; ri < row->length; ri++) {
                //printf("Field: %s Value: %s\n", row->cols[ri]->name,row->cols[ri]->value);
            }
        }
        cur_free_result(&app);

        
        // loop each row and output some json
        cur_query(&app,"SELECT * FROM people");
        json_t * jrow = NULL;
        db_row_t * row2 = NULL;
        while ( cur_next_as_json(app,&jrow,&row2) == CUR_OK ) {
            char * obj = json_dumps(jrow,0);
            //printf("%s\n",obj);
            free(obj);
        }
        cur_free_result(&app);
        
        // Get the whole result set as a json_array
        cur_query(&app,"SELECT * FROM people");
        json_t * all_rows = NULL;
        cur_result_as_json(&app,&all_rows);
        char * rows_string = json_dumps(all_rows,0);
        //printf("%s\n",rows_string);
        free(rows_string);

        json_t * find_test = cur_find_by(&app,"people","first_name","jason",NULL);
        //printf("%s\n",json_dumps(find_test,0));
        json_t * find_sql = cur_find_by_sql(&app,"SELECT * FROM %s WHERE %s = %s","people","first_name","'jason'",NULL);
        printf("Returned json is: %s\n",json_dumps(find_sql,0));
        cur_call_handler(app,event);
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
