#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <curmudgeon.h>
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
    cur->conn = NULL;
    *dest = cur;
    return CUR_OK;
}
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
    // If we are connected to the database, we
    // should close out that connection.
    if ( cur->conn ) {
        if (cur->conn->connected == 1) {
            cur_disconnect(cur);
        }
        free(cur->conn);
    }
    free(cur->events);
    free(cur);
    return CUR_OK;
}
int cur_parse_request( const char * url, event_t ** dest ) {
    event_t * e;
    e = malloc(sizeof(event_t));
    int i;
    int argc = 0;
    int length = strlen(url);
    e->full_url = malloc(sizeof(char) * length + 1);
    memcpy(e->full_url,url,length + 1);
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
int cur_call_handler( curmudgeon_t * cur,event_t * e ) { 
    int i;
    registered_event_t ** events = cur->events;
    for (i = 0; i <= cur->events_length; i++) {
        registered_event_t * re = events[i];
        if (cur_match(&re->regex,e->full_url) == CUR_OK) {
            e->cur = cur;
            return cur->events[i]->callback(e);
        }
    }
    return CUR_HANDLER_NOT_FOUND;
}
int cur_register_event( curmudgeon_t ** dest, char * pattern, int opts, callback_t callback ) {
    curmudgeon_t * cur = *dest;
    cur->events_length++;
    if (cur->events_length >= cur->max_events) {
        cur->events = realloc(cur->events, cur->max_events + 3);
        cur->max_events += 3;
    }
    registered_event_t * re = malloc(sizeof(registered_event_t));
    re->regex = cur_regex(pattern); 
    re->callback = callback;
    cur->events[cur->events_length] = re;
    return CUR_OK;
}
int cur_list_events( curmudgeon_t ** cur ) {
    
    return CUR_OK;
}


// mysql stuff

int cur_mysql( curmudgeon_t ** dest, char * host, char * user, char * pass) {
    curmudgeon_t * cur = *dest;
    adapter_t * a = malloc(sizeof(adapter_t));
    a->type = CUR_MYSQL;
    a->user = user;
    a->pass = pass;
    a->host = host;
    a->port = 3306;
    a->myconn = mysql_init(NULL);
    if (a->myconn == NULL) {
        SET_MYSQL_ERROR
    }
    a->connected = 0;
    cur->conn = a;
    return CUR_OK;
}
int cur_connect( curmudgeon_t ** dest ) {
    curmudgeon_t * cur = *dest;
    if (cur->conn->type == CUR_MYSQL) {
        adapter_t * a = cur->conn;
        if (mysql_real_connect(a->myconn, a->host, a->user, a->pass, NULL, 0, NULL, 0) == NULL) {
            SET_MYSQL_ERROR
        } else {
            a->connected = 1;
            a->myresult = NULL;
        }
    }
    return CUR_OK;
}
int cur_disconnect( curmudgeon_t * cur) {
    if (cur->conn->type == CUR_MYSQL) {
        mysql_close(cur->conn->myconn);
        cur->conn->connected = 0;
    }
    return CUR_OK;
}
int cur_exec( curmudgeon_t ** dest, char * query) {
    curmudgeon_t * cur = *dest;
    adapter_t * a = cur->conn;
    assert(a->connected == 1); 
    if (a->type == CUR_MYSQL) {
        if ( mysql_query(a->myconn,query) != 0) {
           SET_MYSQL_ERROR 
        }
    }
    return CUR_OK;
}
int cur_select_db( curmudgeon_t ** dest, char * db) {
    curmudgeon_t * cur = *dest;
    adapter_t * a = cur->conn;
    assert(a->connected == 1); 
    if (a->type == CUR_MYSQL) {
        if ( mysql_select_db(a->myconn,db) != 0) {
           SET_MYSQL_ERROR 
        }
    }
    return CUR_OK;
}
int cur_query( curmudgeon_t ** dest, char * query) {
    curmudgeon_t * cur = *dest;
    adapter_t * a = cur->conn;
    assert(a->connected == 1); 
    assert(a->myresult == NULL);
    if (a->type == CUR_MYSQL) {
        if ( mysql_query(a->myconn,query) != 0) {
           SET_MYSQL_ERROR 
        } else {
            a->myresult = mysql_store_result(a->myconn);
        }
    }
    return CUR_OK;
}
int cur_free_result( curmudgeon_t ** dest ) {
    curmudgeon_t * cur = *dest;
    adapter_t * a = cur->conn;
    assert(a->connected == 1); 
    assert(a->myresult != NULL);
    if (a->type == CUR_MYSQL) {
        mysql_free_result(a->myresult);
        a->myresult = NULL;
    }
    return CUR_OK;
}
int cur_next( curmudgeon_t * cur, db_row_t ** dest ) {
    db_row_t * row = *dest;
    adapter_t *     a = cur->conn;
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
        free(row->cols);
    }
    row->cols = malloc(sizeof(db_col_t) * num_fields);
    assert(row->cols != NULL);
    row->length = num_fields;
    i = 0;
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
        free(row->field_names);
        free(row->cols);
        free(row);
        return CUR_DB_DONE;
    }
}
int cur_next_as_json( curmudgeon_t * cur, json_t ** dest,db_row_t ** rdest) {
    int i = 0;
    json_t * obj = json_object();
    assert(obj != NULL);
    if (cur_next(cur,rdest) == CUR_OK) {
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
int cur_result_as_json( curmudgeon_t ** cdest, json_t ** odest) {
        json_t * jrow = NULL;
        db_row_t * row2 = NULL;
        json_t * arr = json_array();
        while ( cur_next_as_json(*cdest,&jrow,&row2) == CUR_OK ) {
            json_array_append(arr,jrow);
        }
        cur_free_result(cdest);
        *odest = arr;
        return CUR_OK;
}
json_t * cur_find_by(curmudgeon_t ** cur, char * table, char * field, char * value, char * select) {
    if (!select) {
        select = "*";
    }
    char * fmt = "SELECT %s FROM `%s` WHERE `%s` = '%s'";
    char * query = malloc(strlen(fmt) + strlen(field) + strlen(value) + strlen(table) + strlen(select));
    sprintf(query,fmt,select,table,field,value);
    cur_query(cur,query);
    json_t * all_rows = NULL;
    cur_result_as_json(cur,&all_rows);
    return all_rows;
}
json_t *    cur_find_by_sql(curmudgeon_t ** cur, char * fmt, ...) {
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
    json_t * all_rows = NULL;
    cur_query(cur,query);
    cur_result_as_json(cur,&all_rows);
    return all_rows;
}
int cur_match(regex_t ** reg_dest, char * haystack, ...) {
    va_list iargs;
    va_start(iargs,haystack);
    int offset = va_arg(iargs,int);
    regex_t * re = *reg_dest; 
    int rc;
    int i = 0;
    int j = 0;
    int hlength = strlen(haystack);
    int ovecsize = (re->captc + 1) * 3;
    int * ovector = malloc(sizeof(int) * ovecsize);
    char * value;
    rc = pcre_exec(re->code, re->study, haystack, hlength, 0, 0, ovector, ovecsize); 
    if (rc > 0) {
        re->offset = rc;
        if (re->namec > 0) {
          char * buffer = malloc(sizeof(char) * re->name_entry_size);
          re->named_captures = malloc(sizeof(char*) * (re->namec * 2));
          unsigned char * names = re->names;    
          int nc_index = 0;
          while (i <= re->namec) {
              int index = *(++names); 
              j = 0;
              while(*(++names) != '\0') {
                buffer[j] = *names;  
                j++;
              }
              buffer[j] = '\0';
              if (index > 0) {
                  pcre_get_named_substring(re->code,
                                          haystack, 
                                          ovector,
                                          re->namec, 
                                          buffer,
                                          &value);
                  re->named_captures[nc_index] = strdup(buffer);
                  re->named_captures[nc_index + 1] = strdup(value);
                  nc_index += 2;
              } 
              ++names; 
              i++;
          } // end while  < re->namec 
        } // end if namec > 0
        if (re->captc > 0) {
            re->nummed_captures = malloc(sizeof(char *) * (re->captc + 1));
            for (i = 0; i <= re->captc; i++) {
                pcre_get_substring(haystack, ovector,
                                   (ovecsize / 3), i,
                                   &value); 
                re->nummed_captures[i] = strdup(value);
            }
        }
        return CUR_OK;
    }
    return rc;
}
regex_t * cur_regex(char * pattern) {
    regex_t * re;
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
    if (pattern[0] == '/') {
        i = 1;
        j = 0;
        while (i < pattern_length) {
            parse_top:
            if (pattern[i] == '\\') {
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
    re = malloc(sizeof(regex_t));
    re->pattern = pattern;
    re->options = options;
    re->named = _regex_named;
    re->capt = _regex_nummed;
    re->code = pcre_compile2(buffer,options,&errcodeptr,&errptr, &erroffset,NULL);
    if ( ! re->code ) {
        printf("%d %s\n",errcodeptr,_get_pcre_error(errcodeptr));
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
   return re; 
}

// Private cur functions
//
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
char * _regex_named(regex_t * re,char * n) {
    int i;
    for (i = 0; i < re->namec; i += 2) {
        char * key = re->named_captures[i];
        if (strcmp(key,n) == 0) {
            return re->named_captures[i+1];
        }
    }
    return NULL;
}
char * _regex_nummed(regex_t * re, int num) {
    if ( num <= re->captc ) {
        return re->nummed_captures[num];
    } else {
        return NULL;
    }
}
