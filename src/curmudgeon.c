#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <curmudgeon.h>
// The cur acts as a kind of global store, it holds all
// the events that are registered etc. You can to call
// this function first, and pass a reference to a curmudgeon_t *
//
// That's right, **. Just means a pointer to a point to a value.
int cur_init( curmudgeon_t ** dest, int num_events ) {
    curmudgeon_t * cur;
    cur = malloc(sizeof(curmudgeon_t *));
    if ( ! cur ) {
        return CUR_CUR_ALLOC_FAILED;
    }
    cur->events = malloc(sizeof(registered_event_t *) * num_events); 
    if ( ! cur->events ) {
       free(cur);
       return CUR_EVENTS_ALLOC_FAILED;
    }
    cur->max_events = num_events;
    cur->events_length = -1;
    cur->schema = 0;
    cur->schema_version_file = "conf/schema.version";// this is overrideable
    // need to loadup schema data
    *dest = cur; // notice how we set *dest here, i.e. we overwrite the pointer that points to the value.
    return CUR_OK;
}
// Really just to be good citizens. cur_done should only be called
// before app exit. Just cleanup code. This should not be called
// after each request!
int cur_done( curmudgeon_t ** to_free ) {
    curmudgeon_t * cur = *to_free;
    int i;
    if ( ! cur ) {
        return CUR_CANNOT_FREE;
    }
    if ( ! cur->events) {
        return CUR_CANNOT_FREE; 
    }
    for (i = 0; i <= cur->events_length; i++) {
            free(cur->events[i]);
    }
    return CUR_OK;
}
int cur_free_adapter(adapter_t ** adptr) {
    adapter_t * a = *adptr;
    // If we are connected to the database, we
    // should close out that connection.
    if ( a ) {
        if (a->connected == 1) {
            db_disconnect(a);
        }
    }
    free(a);
    return CUR_OK;
}
// Holding/testing function, the real function will probably be 
// very different.
int cur_parse_request( const char * url, event_t ** dest ) {
    event_t * e;
    e = malloc(sizeof(event_t));
    int i;
    int argc = 0;
    int length = strlen(url);
    e->full_url = url;
    for (i = 1; i < length; i++) {
       if (url[i] == '/') {
           argc++;
       } 
    } 
    e->args = malloc(sizeof(char *) * argc);
    e->argc = argc;
    char * token;
    char * cpy;
    cpy = strdup(url);
    cpy++; // skip the first char
    token = strsep(&cpy,"/");
    e->name = strdup(token);
    i = 0;
    while( ( token = strsep(&cpy,"/")) ) {
        e->args[i] = strdup(token);
        i++;
    }
    *dest = e;
    return CUR_OK;
}
// This just iterates over the registered "events" and then
// runs their regexes against the event->full_url and if it
// matches, it calls the handler. I don't know if we want
// to all multiple handlers per url or just one? I think
// just one is simpler and if you need more, you can just
// call them yourself from a handler.
int cur_call_handler( curmudgeon_t * cur,event_t ** e_dest ) { 
    int i;
    registered_event_t ** events = cur->events;
    event_t * e = *e_dest;
    for (i = 0; i <= cur->events_length; i++) {
        registered_event_t * re = events[i];
        if (cur_match(&re->regex,e->full_url) == CUR_OK) {
            e->cur = cur;
            return cur->events[i]->callback(&e);
        }
    }
    return CUR_HANDLER_NOT_FOUND;
}
// Registering an event. pattern can be in two flavors:
//    1) hello_world[\d]+
//    2) /hello_world[\d]+/ims$j
// That is we support perl/ruby style regex with /.../options
// You'll need to look at the cur_regex function to see about the options
// supported.
int cur_register_event( curmudgeon_t ** dest, char * pattern, int opts, callback_t callback ) {
    curmudgeon_t * cur = *dest;
    cur->events_length++;
    if (cur->events_length >= cur->max_events) {
        cur->events = realloc(cur->events, cur->max_events + 3);
        cur->max_events += 3;
    }
    registered_event_t * re = malloc(sizeof(registered_event_t));
    regex_t * reg = NULL;
    cur_regex(&reg,pattern); 
    re->regex = reg;
    re->callback = callback;
    cur->events[cur->events_length] = re;
    return CUR_OK;
}
int cur_list_events( curmudgeon_t ** cur ) {
    
    return CUR_OK;
}
// Actually executes the regex match, filling out any data
// on the regex struct for later use.
int cur_match(regex_t ** reg_dest, char * haystack, ...) {
    va_list iargs;
    va_start(iargs,haystack);
    int offset = va_arg(iargs,int);
    regex_t * re = *reg_dest; 
    if (re->used == 1) {
    
    }
    int rc;
    int i = 0;
    int j = 0;
    int hlength = strlen(haystack);
    int ovecsize = (re->captc + 1) * 3;
    re->ovector = malloc(sizeof(int) * ovecsize);
    char * value;
    rc = pcre_exec(re->code, re->study, haystack, hlength, 0, 0, re->ovector, ovecsize); 
    if (rc > 0) {
        re->used = 1;
        re->haystack = haystack;
        return CUR_OK;
    }
    return rc;
}
// The grandaddy regex function. Lots of crazy shit is
// going on here specific to interfacing with PCRE which
// requires reading man pcreapi which has all the info
// you would need. In reality it's a bit straightforward,
// it's just an antiquated c api. 
int cur_regex(regex_t ** re_dest,char * pattern) {
    char * errptr = malloc(sizeof(char) *1024);
    int erroffset;
    int errcodeptr = 0;
    int options = 0;
    int i = 0;
    int j = 0;
    int rc = 0;
    int pattern_length = strlen(pattern);
    char * buffer = malloc(sizeof(char) * pattern_length); 
    int    buffer_length = 0;
    // So here we see if the regex is of type two, i.e. like PERL or Ruby
    // where is starts and ends with / and has options like /isg etc
    if (pattern[0] == '/') {
        i = 1;
        j = 0;
        while (i < pattern_length) {
            parse_top:
            if (pattern[i] == '-') {
             buffer[j] = pattern[++i]; //i.e. go to next char and append
             j++; // next position in buffer
             i++; // next character
             goto parse_top;
            } 
            if (pattern[i] == '/') {
                // we are at the end of the pattern
                buffer[j] = '\0';
                i++; // get ready for parsing the options;
                break;
            }
            buffer[j] = pattern[i];
            j++;
            i++;
        }
        buffer_length = j; 
        // Now we need to get the options
        while (i < pattern_length) {
            char opt = pattern[i];
            switch(opt) {
                case 'a':
                    options |= PCRE_ANCHORED;
                    break;
                case 'n':
                    options |= PCRE_BSR_ANYCRLF;
                    break;
                case 'i':
                    options |= PCRE_CASELESS;
                    break;
                case '$':
                    options |= PCRE_DOLLAR_ENDONLY;
                    break;
                case 's':
                    options |= PCRE_DOTALL;
                    break;
                case 'x':
                    options |= PCRE_EXTENDED;
                    break;
                case 'j':
                    options |= PCRE_JAVASCRIPT_COMPAT;
                    break;
                case 'm':
                    options |= PCRE_MULTILINE;
                    break;
                default:
                    // i.e. we break out of the loop if we got an insane
                    // option. cause fuck'em if they don't read the docs.
                    // maybe we should throw and error here?
                    i = pattern_length;
                    break;
            }
            i++;
        }
        // Now we have our pattern in buffer, our options are set, so it's time
        // to compile the regex
    } else {
        // Treat this like a direct pattern
        buffer = pattern;
    }
    regex_t * re = *re_dest;
    // we need to institute a null check here so
    // that we can reuse the regex.
    if ( ! re) {
        re = malloc(sizeof(regex_t));
    } else {
        free(re->pattern);
        pcre_free(re->code);
        if (re->study) {
            pcre_free(re->study);
        }
    }
    re->pattern = strdup(pattern);
    re->options = options;
    re->named = _regex_named; // these are pointers to functions that can be
    // overridden by the user.
    re->capt = _regex_nummed;
    re->study = NULL;
    re->used = 0;
    // There's really no need to be using compile 2 here...just in case
    // we want to return errcodeptr.
    re->code = pcre_compile2(buffer,options,&errcodeptr,&errptr, &erroffset,NULL);
    if ( ! re->code ) {
        // but we just die if it fails
        printf("Your regex would not compile: %d %s\n",errcodeptr,_get_pcre_error(errcodeptr));
        assert(1 == 0);
    } else {
        //okay, we've compiled the regex, now it's time to study it
        re->study = pcre_study(re->code, 0,&errptr);
        rc = pcre_fullinfo(re->code,re->study,PCRE_INFO_BACKREFMAX, &re->backrefc);     
        rc = pcre_fullinfo(re->code,re->study,PCRE_INFO_CAPTURECOUNT, &re->captc);     
        rc = pcre_fullinfo(re->code,re->study,PCRE_INFO_NAMECOUNT, &re->namec);     
        rc = pcre_fullinfo(re->code,re->study,PCRE_INFO_NAMEENTRYSIZE, &re->name_entry_size);     
        rc = pcre_fullinfo(re->code,re->study,PCRE_INFO_NAMETABLE, &re->names);     
    }
    // because we might have malloced so we need to ensure that the pointer is updated
    // we only malloc when we were passed NULL, so this should not be leaking.
    *re_dest = re;
   return CUR_OK;
}
// needs some more planning to avoid memory leaks
int cur_free_regex(regex_t ** re_dest) {
    regex_t * re = *re_dest;
    if (re->study) {
        pcre_free(re->study);
        re->study = NULL;
    }
    if (re->code) {
        pcre_free(re->code);
        re->code = NULL;
    }
    free(re);
    re = NULL;
    *re_dest = re;
    return CUR_OK;
}

// mysql stuff
// So the idea is that even though we only support mysql, we should make it
// easy to support more than one db. In some cases people will use
// mysql for one db, and maybe some nosql piece of shit for a message
// queue or some such. They should be able to use these simultaneously.
//
// I am not sure if this code is very re-entrant/thread safe, so we'll
// have to think about that.
//
// curmudgeon_t * should probably have some form of mutex because the
// result is store on that structure. otherwise, you need to pass
// around and keep track of a lot of different values and
// be freeing them yourself and we wouldn't be much of a
// framework if we made the user do absolutely everything.
int db_mysql( curmudgeon_t * cur,adapter_t ** adptr, char * host, char * user, char * pass) {
    adapter_t * a = malloc(sizeof(adapter_t));
    a->type = CUR_MYSQL;
    a->user = user;
    a->pass = pass;
    a->host = host;
    a->port = 3306;
    a->myconn = mysql_init(NULL);
    a->schema = cur->schema;
    a->schema_version_file = cur->schema_version_file;
    if (a->myconn == NULL) {
        SET_MYSQL_ERROR
    }
    a->connected = 0;
    *adptr = a;
    return CUR_OK;
}
int db_connect( adapter_t * a ) {
    if (a->type == CUR_MYSQL) {
        if (mysql_real_connect(a->myconn, a->host, a->user, a->pass, NULL, 0, NULL, 0) == NULL) {
            // this is a macro that prints out the error and returns CUR_DB_ERROR.
            SET_MYSQL_ERROR
        } else {
            a->connected = 1;
            a->myresult = NULL;
        }
    }
    return CUR_OK;
}
int db_disconnect( adapter_t * a) {
    if (a->type == CUR_MYSQL) {
        mysql_close(a->myconn);
        a->connected = 0;
    }
    return CUR_OK;
}
int db_exec( adapter_t * a, char * query) {
    assert(a->connected == 1); 
    if (a->type == CUR_MYSQL) {
        if ( mysql_query(a->myconn,query) != 0) {
           SET_MYSQL_ERROR 
        }
    }
    return CUR_OK;
}
int db_select_db( adapter_t * a, char * db) {
    assert(a->connected == 1); 
    if (a->type == CUR_MYSQL) {
        if ( mysql_select_db(a->myconn,db) != 0) {
           SET_MYSQL_ERROR 
        }
    }
    return CUR_OK;
}
int db_query( adapter_t * a, char * query) {
    assert(a->connected == 1); 
    assert(a->myresult == NULL);
    if (a->type == CUR_MYSQL) {
        if ( mysql_query(a->myconn,query) != 0) {
           SET_MYSQL_ERROR 
        } else {
            // here's the magic, the result pointer is
            // stored on the curmudgeon_t * struct's adapter member
            a->myresult = mysql_store_result(a->myconn);
        }
    }
    return CUR_OK;
}
int db_free_result( adapter_t * a ) {
    assert(a->connected == 1); 
    assert(a->myresult != NULL);
    if (a->type == CUR_MYSQL) {
        mysql_free_result(a->myresult);
        a->myresult = NULL; // remember to set stuff to null so we can check it
    }
    return CUR_OK;
}
void _free_db_row_cols(db_row_t * row) {
    int i;
    for (i = 0; i < row->length; i++) {
        if (row->cols[i]) {
            free(row->cols[i]);
        }
    }
}
int db_next( adapter_t * a, db_row_t ** dest ) {
    db_row_t * row = *dest;
    assert(a->connected == 1); 
    assert(a->myresult != NULL);
    MYSQL_ROW       mrow;
    MYSQL_FIELD *   field;
    int             num_fields = 0;
    int             i = 0;
    num_fields = mysql_num_fields(a->myresult);
    if (!row) { 
        row = malloc(sizeof(db_row_t));
        row->cols = NULL;
        row->field_names = malloc(sizeof(char *) * num_fields);
    }
    assert(row != NULL);
    if (row->cols) {
        _free_db_row_cols(row);
    }
    row->cols = malloc(sizeof(db_col_t) * num_fields);
    assert(row->cols != NULL);
    row->length = num_fields;
    // Null all of the entries!
    for (i = 0; i < row->length; i++) {
        row->cols[i] = NULL;
    }
    i = 0;
    // this code will only execute the first time you call
    // next. Because after you have fetched all the rows
    // it will return null, which is why we store the row
    // names on row->field_names
    while ( (field = mysql_fetch_field(a->myresult)) ) {
       row->field_names[i] = strdup(field->name);
       i++;
    }
    if ( (mrow = mysql_fetch_row(a->myresult)) ) {
        for (i = 0; i < num_fields; i++) {
            db_col_t * col = malloc(sizeof(db_col_t));
            assert(col != NULL);
            col->name = row->field_names[i]; 
            if (mrow[i]) {
                col->value = strdup(mrow[i]);
            } else {
                col->value = "NULL";
            }
            row->cols[i] = col;
        } 
        *dest = row;
        return CUR_OK;
    } else {
        // need to avoid mem leaks here so
        // we free the strdupped field names
        for (i = 0; i < row->length; i++) {
            free(row->field_names[i]);
        }
        free(row->field_names);
        _free_db_row_cols(row); 
        free(row->cols);
        free(row);
        // we should probably free the result here automatically?
        return CUR_DB_DONE;
    }
}
// This function just proxies db_next and packages the row up
// as a json object like so: {f1:v1 ...}
int db_next_as_json( adapter_t * a, json_t ** dest,db_row_t ** rdest) {
    int i = 0;
    json_t * obj = json_object();
    assert(obj != NULL);
    if (db_next(a,rdest) == CUR_OK) {
        db_row_t * row = *rdest;
        for (i = 0; i < row->length; i++) {
            json_t * field = json_string(row->cols[i]->value);
            assert(field != NULL);
            json_object_set(obj,row->cols[i]->name,field);
        } 
        *dest = obj;
        return CUR_OK;
    } else {
        return CUR_DB_DONE;
    }
}
// Same as above, except this snags the complete result set.
// so if you request 10,000 rows, you'll get a json array
// of 10,000 objects!
int db_result_as_json( adapter_t * a, json_t ** odest) {
        json_t * jrow = NULL;
        db_row_t * row2 = NULL;
        json_t * arr = json_array();
        while ( db_next_as_json(a,&jrow,&row2) == CUR_OK ) {
            json_array_append(arr,jrow);
        }
        db_free_result(a);
        *odest = arr;
        return CUR_OK;
}
int db_find_by(adapter_t * a, json_t ** all_rows,char * table, char * field, char * value, char * select) {
    if (!select) {
        select = "*";
    }
    char * fmt = "SELECT %s FROM `%s` WHERE `%s` = '%s'";
    char * query = malloc(strlen(fmt) + strlen(field) + strlen(value) + strlen(table) + strlen(select));
    sprintf(query,fmt,select,table,field,value);
    db_query(a,query);
    db_result_as_json(a,all_rows);
    return CUR_OK;
}
int db_find_by_sql(adapter_t * a,json_t ** all_rows, char * fmt, ...) {
    va_list sargs;
    va_start(sargs,fmt);
    char * tmp_str;
    int char_count = strlen(fmt);
    while( (tmp_str = va_arg(sargs,char *)) ) {
        char_count += strlen(tmp_str); 
    }
    char * query = malloc(sizeof(char) * char_count);
    va_start(sargs,fmt);
    vsprintf(query,fmt,sargs);
    db_query(a,query);
    db_result_as_json(a,all_rows);
    return CUR_OK;
}
// Schema Functions

int schema_database(adapter_t * a, char * database, char * charset,char * collate) {
    int version = _get_num_from_file(a->schema_version_file);
    if (version < a->schema || version == 0) {
       if (a->type == CUR_MYSQL) {
           if ( ! charset ) {
               charset = "utf8";
           }
           if ( ! collate ) {
               collate = "utf8_general_ci";
           }
           char * fmt = "CREATE DATABASE IF NOT EXISTS `%s` DEFAULT CHARACTER SET %s DEFAULT COLLATE %s";
           int len = strlen(fmt) + strlen(charset) + strlen(collate) + 3;
           char * query = malloc(sizeof(char) * len);
           sprintf(query,fmt,database,charset,collate);
           printf("query is: %s\n",query);
           a->database_name = database;
       }
       return CUR_OK;
    } else {
        return CUR_SCHEMA_UPTODATE;
    }
}
/*
 * the ... should be: row_id_name, row_id_options*/
int schema_table(adapter_t * adptr, char * table, ...) {
    va_list sargs;
    va_start(sargs,table);
    char * tmp_str;
    char * row_id_name = "_rowid_";
    char * row_id_options = "BIGINT NOT NULL AUTO_INCREMENT";
    char * engine = "MyISAM";
    int i = 0;
    while( (tmp_str = va_arg(sargs,char *)) ) {
        switch(i) {
            case 0:
                row_id_name = strdup(tmp_str);
                break;
            case 1:
                row_id_options = strdup(tmp_str);
                break;

            default:
                break;
        } 
        i++;
    }
    return CUR_OK;
}
// Curmudgeon JSON Options
cur_opts_t * cur_create_options(char * options_string) {
    char * fmt = "{%s}";
    int len = strlen(fmt) + strlen(options_string) + 1;
    char * opts = malloc(sizeof(char) * len);
    sprintf(opts,fmt,options_string);
    json_t * obj = _decode_json(opts);
    cur_opts_t * final = malloc(sizeof(cur_opts_t));
    final->original_string = strdup(options_string);
    final->json = obj;
    return final;
}
int cur_options(cur_opts_t * opts,char * key, char ** value) {
    json_t * result = _drill_down(opts->json,key);
    if (json_is_string(result)) {
        *value = strdup(json_string_value(result));
         if (! *value ) {
            return CUR_WRONG_TYPE;
         }
    }
    return CUR_OK;
}

// Private cur functions
//
json_t * _decode_json(char * str) {
    json_error_t error;
    printf("str is: -- %s --\n",str);
    json_t * obj = json_loads(str,JSON_DECODE_ANY,&error);
    if (! obj ) {
        printf("JSON Error: %s at line %d col %d\n",error.text, error.line,error.column);
    }
    return obj;
}

json_t * _drill_down(json_t * obj, char *key) {
    char * cpy = strdup(key);
    char * token = strsep(&cpy,".");
    json_t * current_obj = json_object_get(obj,token);
    while ( (token = strsep(&cpy,".")) ) {
        current_obj = json_object_get(current_obj,token); 
    }
    return current_obj;
}

char * _get_pcre_error(int code) {
    switch (code) {

        case 0:
            return "no error";
            break;

        case 1:
            return "\\ at end of pattern";
            break;

        case 2:
            return "\\c at end of pattern";
            break;

        case 3:
            return "unrecognized character follows \\";
            break;

        case 4:
            return "numbers out of order in {} quantifier";
            break;

        case 5:
            return "number too big in {} quantifier";
            break;

        case 6:
            return "missing terminating ] for character class";
            break;

        case 7:
            return "invalid escape sequence in character class";
            break;

        case 8:
            return "range out of order in character class";
            break;

        case 9:
            return "nothing to repeat";
            break;

        case 10:
            return "[this code is not in use]";
            break;

        case 11:
            return "internal error: unexpected repeat";
            break;

        case 12:
            return "unrecognized character after (? or (?-";
            break;

        case 13:
            return "POSIX named classes are supported only within a class";
            break;

        case 14:
            return "missing )";
            break;

        case 15:
            return "reference to non-existent subpattern";
            break;

        case 16:
            return "erroffset passed as NULL";
            break;

        case 17:
            return "unknown option bit(s) set";
            break;

        case 18:
            return "missing ) after comment";
            break;

        case 19:
            return "[this code is not in use]";
            break;

        case 20:
            return "regular expression is too large";
            break;

        case 21:
            return "failed to get memory";
            break;

        case 22:
            return "unmatched parentheses";
            break;

        case 23:
            return "internal error: code overflow";
            break;

        case 24:
            return "unrecognized character after (?<";
            break;

        case 25:
            return "lookbehind assertion is not fixed length";
            break;

        case 26:
            return "malformed number or name after (?(";
            break;

        case 27:
            return "conditional group contains more than two branches";
            break;

        case 28:
            return "assertion expected after (?(";
            break;

        case 29:
            return "(?R or (?[+-]digits must be followed by )";
            break;

        case 30:
            return "unknown POSIX class name";
            break;

        case 31:
            return "POSIX collating elements are not supported";
            break;

        case 32:
            return "this version of PCRE is not compiled with PCRE_UTF8 support";
            break;

        case 33:
            return "[this code is not in use]";
            break;

        case 34:
            return "character value in \\x{...} sequence is too large";
            break;

        case 35:
            return "invalid condition (?(0)";
            break;

        case 36:
            return "\\C not allowed in lookbehind assertion";
            break;

        case 37:
            return "PCRE does not support \\L, \\l, \\N, \\U, or \\u";
            break;

        case 38:
            return "number after (?C is > 255";
            break;

        default:
            return "Unknown Error";
            break;
    }
    return "Unknow error";
}
// Here we have simplified finding the key/value for named
// captures by just parsing out the index from PCRE's table
// in memory. This way we worry less about causing a memory
// leak or allocating new memory for stuff that's already there.
char * _regex_named(regex_t * re,char * n) {
    if (re->namec > 0) {
          char * buffer = malloc(sizeof(char) * re->name_entry_size);
          unsigned char * names = re->names;    
          int nc_index = 0;
          int i,j;
          i = j = 0;
          while (i <= re->namec) {
              int index = *(++names); 
              j = 0;
              // We need to do this because of the unsigned charness of
              // the entry in the table. It makes it harder to search and
              // compare. 
              while(*(++names) != '\0') {
                  buffer[j] = *names;
                  j++;
              }
              buffer[j] = '\0';
              if (index > 0) {
                if (strcmp(buffer,n) == 0) {
                    // All of that work just to find the index and name entry
                    // now we just call the capt function which is
                    // defined below this function and be done with it :)
                    return re->capt(re,index); 
                }    
              } 
              ++names; 
              i++;
          } // end while  < re->namec 
        } // end if namec > 0
    return NULL;
}
char * _regex_nummed(regex_t * re, int num) {
    // if we put this first, then the second list can
    // point to entries here
    char * value = NULL;
    if (re->captc > 0) {
        pcre_get_substring(re->haystack, re->ovector,
                           re->captc+1, num,
                           &value); 
    }
   return value; 
}
// this was created during a free error search, but it didn't help
// so it can be deprecated. 
char * _strdup (char * src) {
    char * ret = malloc(sizeof(char) * ( strlen(src) + 4 ) );
    strcpy(ret,src);
    return ret;
}
int _compare(const char * s1, const char * s2) {
    int i;
    int l = strlen(s2);
    for (i = 0; i < l; i++) {
        if (s1[i] != s2[i]) {
            return -1;
        }
    }
    return 0;
}
int _get_num_from_file(char * filename) {
#ifdef DEBUG
    printf("Getting file: %s\n",filename);
#endif
    FILE * fp = fopen(filename,"r");
    if ( ! fp ) {
     printf("Failed to open %s in _get_num_from_file\n",filename); 
     assert(1 == 0);
    }
    char c;
    char buffer[11]; // Max int is: 2147483647 that's 11 chars + nul
    int cap = 11; // i.e. we allow 0-10 (i.e. 11) chars
    int i = 0;
    while ( ( (c = fgetc(fp)) != EOF ) && i < cap ) {
        buffer[i] = c;
        i++;
    }
    buffer[++i] = '\0'; // then we append a \0
    fclose(fp);
    return atoi(buffer);
}
int _set_num_in_file(char * filename,int num) {
    FILE * fp = fopen(filename,"w");
    if ( ! fp ) {
     printf("Failed to open %s in _set_num_in_file \n",filename); 
     assert(1 == 0);
    }
    char buffer[11];
    sprintf(buffer,"%d",num);
    fwrite(buffer,strlen(buffer), sizeof(char),fp);
    fclose(fp);
    return CUR_OK;
}
