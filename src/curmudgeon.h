#ifndef CURMUDGEON_H
#define CURMUDGEON_H
/*
 * curmudgeon is an ER framework, i.e. Event -> Response
 * You register events (urls) and a pointer to a c function
 * that accepts the event_t and then you do stuff when it
 * happens. This isn't rocket science.
 *
 * */
#include <my_global.h>
#include <mysql.h>
#include <jansson.h>
#include <assert.h>
#include <pcre.h>
#include <iconv.h>
#define SET_MYSQL_ERROR a->last_error = strdup(mysql_error(a->myconn)); \
            a->last_errno = mysql_errno(a->myconn); \
            printf("You have MYSQL errors: (%d) -- %s \n",a->last_errno,a->last_error); \
            return CUR_DB_ERROR; \

enum cur_return_codes {
    CUR_OK,
    CUR_CUR_ALLOC_FAILED,
    CUR_EVENTS_ALLOC_FAILED,
    CUR_CANNOT_FREE,
    CUR_HANDLER_NOT_FOUND,
    CUR_DB_ERROR,
    CUR_DB_DONE,
    CUR_SCHEMA_UPTODATE,
    CUR_WRONG_TYPE,
    CUR_JSON_ERROR
};
typedef struct curmudgeon_options cur_opts_t;
typedef struct event event_t;
typedef struct adapter adapter_t;
typedef struct regex regex_t;
typedef int (*callback_t)(event_t **);
typedef struct registered_event {
    regex_t * regex;
    callback_t  callback;
} registered_event_t;

typedef struct curmudgeon {
    int max_events;
    int events_length;
    registered_event_t **events;
    int schema; // Your database schema version
    char *schema_version_file;
} curmudgeon_t;
struct event {
    char *name; // the even name, i.e. mydomain.com/event/arg1/arg2
    char *full_url;
    char *ip;
    int argc;
    char **args;
    curmudgeon_t * cur;
};
struct regex {
    // This is the pattern, we keep it for some
    // calls to pcre which use the pattern sometimes
    // as a kind of reference...
    char *pattern;
    // This is the code, possibly jitted by
    // pcre that contains the automaton for
    // matching the regex
    pcre * code;
    // we pre-study all of the pcre regexes
    // this just collects information etc
    pcre_extra * study;
    // We also extract some info from pcre
    // for future calls
    int backrefc; //back ref count
    int captc; // capture count
    int namec; // named capt count
    int name_entry_size; // largest name
    unsigned char *names;
    int options; // options used
    // a pcre list that contains to capture offsets
    int * ovector;
    short used; // just a tag to see if this has already been used
    // if so, we shouldn't allocate stuff.
    char *haystack; // used when getting captures because those
    // are only stored as offsets. This should be a pointer to the
    // original string passed in. If that string is freed, we're
    // fucked, but that's up to the user.
    //
    // These are overriddable functions I think we should/could expand
    // the system to allow overridding most functions.
    char * (*named)(regex_t *,char *); 
    char * (*capt)(regex_t *,int); 
};
struct curmudgeon_options {
    char *original_string;
    json_t * json;
};
int              cur_init( curmudgeon_t **cur, int num_events ); // called at startup
int              cur_done( curmudgeon_t **cur );               // called to clean up when done.
int              cur_free_adapter(adapter_t **adptr);
int              cur_parse_request( const char *url, event_t **e );
int              cur_call_handler( curmudgeon_t * cur, event_t **e );
int              cur_register_event( curmudgeon_t **cur, char *pattern, int opts, callback_t callback );
int              cur_list_events( curmudgeon_t **cur );
int              cur_match(regex_t **regex,char *haystack,...);
int              cur_regex(regex_t **,char *pattern);
int              cur_free_regex(regex_t **);



// Database related stuff
enum cur_adapters {
    CUR_MYSQL
};

/**
 * The Adapter is the connection to the database,
 * and also where we store results.
 * */
struct adapter {
    short           connected;
    int             type;
    char *         user;
    char *         pass;
    char *         host;
    unsigned int    port;
    unsigned long   flags;
    char *         last_error;
    char *         database_name;
    unsigned int    last_errno;
    int schema;
    char *schema_version_file;
    union {
        MYSQL * myconn;
    };
    union {
        MYSQL_RES * myresult;
    };
};
typedef struct db_row db_row_t;
typedef struct db_col db_col_t;
struct db_row {
    db_col_t **cols;
    char **field_names;
    int length;
};
struct db_col {
    char *name;
    char *value;
    int value_length;
    int name_length;
};
            // We need a pointer to the adapter to do the mallocs
int         db_mysql( curmudgeon_t * cur,adapter_t **adptr, char *host, char *user, char *pass);
int         db_connect( adapter_t * adptr );
int         db_disconnect( adapter_t * adptr);
int         db_exec( adapter_t * adptr, char *query );
int         db_select_db( adapter_t * adptr, char *db);
int         db_query(adapter_t * adptr, char *query);
int         db_free_result( adapter_t * adptr );
int         db_next( adapter_t * adptr, db_row_t **row );
int         db_next_as_json( adapter_t * adptr, json_t **obj,db_row_t **);
int         db_result_as_json( adapter_t * adptr, json_t **obj);
int         db_find_by(adapter_t * adptr, json_t **, char *table, char *field, char *value, char *select);
int         db_find_by_sql(adapter_t * adptr, json_t **, char *query, ...);
/**
 * Schema and migration functions. The basic idea is that there is a pickup
 * file in the app/conf directory called schema.version. When we call one
 * of these schema functions, we need to check that file against
 * the current app->schema version. If it is <= we do nothing, otherwise
 * we run the query.
 *
 * The pickup file path is defined in app->schema_version_file and
 * can be set by the app to point somewhere else.
 * */
int        schema_database(adapter_t * adptr, char *database, char *charset,char *collate);
int        schema_table(adapter_t * adptr, char *table, ...);

/**
 * General Framework Options API, AKA Helpers
 * These functions generally return a pointer to
 * a pre-initialized and allocated object or
 * value that you will have to free yourself,
 * or via a freeing helper
 * */

/**
 * We support passing options to c functions via a
 * JSON object. At first we were trying to use
 * variadic functions, but that's a scary place
 * and prone to error. Instead, the idea is that
 * a method has default arguments, and any argument
 * that is not provided by the JSON object receives
 * a default value.
 *
 * If a function for say a query, requires a database,
 * and a table attribute, the you would do this
 * cur_opts_t * opts = cur_new_options("table: 'mytable',database:'mydatabase'");
 * cur_sql_func(adptr,opts); // Not a real function,
 * just an example.
 *
 * { and } will automatically be added for you.
 *
 * Any level or complexity of options is supported.
 * 
 * Generally speaking, these opts should be
 * static so that they are only compiled once.
 *
 * If you need dyname values, then you can set
 * the value of the key after you have compiled
 * the JSON.
 *
 * You might try something like:
 *
 * static cur_opts_t * opts = cur_new_options("table: '', database:''");
 * cur_set_options(opts,"table","my_table");
 * cur_set_options(opts,"database","my_database");
 *
 * Note the key is in format key.key.key
 *
 * So if you have {database: {name: '', table: ''}}
 *
 * Then you would use:
 *
 * cur_set_options(opts,"database.name","my_database");
 * cur_set_options(opts,"database.table","my_table");
 *
 * Notice the 's' at the end.
 *
 * s for string.
 *
 * cur_set_optioni => int
 * cur_set_optiond => double
 * cur_set_optiono => json_t
 * cur_set_options => char *
 * */
cur_opts_t *     cur_create_options(char *options_string);
/**
 * key must be a null string, as it will be allocated for you
 * */
int              cur_options(cur_opts_t *opts,char *key, char **value);
int              cur_options_set(cur_opts_t *opts, char *key, char *value);
// Private internal functions, not really intended
// to be used by app developers, though they can if
// they want, these functions don't really clean up
// after themselves, they just do a job and return
json_t *         _json_decode(char *str);
                // Will take a key like: l1.l2.l3 and return l3
json_t *         _json_drill_down(json_t *obj, char *key);
void             _json_create_key_chain(json_t *opts, char *key, char type);
int              _get_num_from_file(char *filename);
int              _set_num_in_file(char *filename,int num);
char *           _get_pcre_error(int code);
char *           _regex_nummed(regex_t *re,int i);
char *           _regex_named(regex_t *re,char *n);
char *           _strdup (char *src);
int              _compare(const char *s1, const char *s2); 
#endif
