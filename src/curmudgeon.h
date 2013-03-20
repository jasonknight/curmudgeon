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
    CUR_DB_DONE
};

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
    registered_event_t ** events;
    adapter_t * conn;
} curmudgeon_t;
struct event {
    char * name; // the even name, i.e. mydomain.com/event/arg1/arg2
    char * full_url;
    char * ip;
    int argc;
    char ** args;
    curmudgeon_t * cur;
};
struct regex {
    // This is the pattern, we keep it for some
    // calls to pcre which use the pattern sometimes
    // as a kind of reference...
    char * pattern;
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
    unsigned char * names;
    int options; // options used
    // a pcre list that contains to capture offsets
    int * ovector;
    short used; // just a tag to see if this has already been used
    // if so, we shouldn't allocate stuff.
    char * haystack; // used when getting captures because those
    // are only stored as offsets. This should be a pointer to the
    // original string passed in. If that string is freed, we're
    // fucked, but that's up to the user.
    //
    // These are overriddable functions I think we should/could expand
    // the system to allow overridding most functions.
    char *  (*named)(regex_t *,char *); 
    char *  (*capt)(regex_t *,int); 
};
int              cur_init( curmudgeon_t ** cur, int num_events ); // called at startup
int              cur_done( curmudgeon_t ** cur );               // called to clean up when done.
int              cur_parse_request( const char * url, event_t ** e );
int              cur_call_handler( curmudgeon_t * cur, event_t ** e );
int              cur_register_event( curmudgeon_t ** cur, char * pattern, int opts, callback_t callback );
int              cur_list_events( curmudgeon_t ** cur );
int              cur_match(regex_t ** regex,char * haystack,...);
int              cur_regex(regex_t **,char * pattern);
int              cur_free_regex(regex_t **);



// Database related stuff
enum cur_adapters {
    CUR_MYSQL
};

struct adapter {
    short           connected;
    int             type;
    char *          user;
    char *          pass;
    char *          host;
    unsigned int    port;
    unsigned long   flags;
    char *          last_error;
    unsigned int    last_errno;
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
    db_col_t ** cols;
    char ** field_names;
    int length;
};
struct db_col {
    char * name;
    char * value;
    int value_length;
    int name_length;
};

int         db_mysql( curmudgeon_t ** cur, char * host, char * user, char * pass);
int         db_connect( curmudgeon_t ** cur );
int         db_disconnect( curmudgeon_t * cur);
int         db_exec( curmudgeon_t ** cur, char * query );
int         db_select_db( curmudgeon_t ** cur, char * db);
int         db_query( curmudgeon_t ** cur, char * query);
int         db_free_result( curmudgeon_t ** cur );
int         db_next( curmudgeon_t * cur, db_row_t ** row );
int         db_next_as_json( curmudgeon_t * cur, json_t ** obj,db_row_t **);
int         db_result_as_json( curmudgeon_t ** cur, json_t ** obj);

int         db_find_by(curmudgeon_t ** cur, json_t **, char * table, char * field, char * value, char * select);
int         db_find_by_sql(curmudgeon_t ** cur, json_t **, char * query, ...);

// Private internal functions, not really intended
// to be used by app developers, though they can if
// they want, these functions don't really clean up
// after themselves, they just do a job and return
char *           _get_pcre_error(int code);
char *           _regex_nummed(regex_t * re,int i);
char *           _regex_named(regex_t * re,char * n);
char *           _strdup (char * src);
int              _compare(const char * s1, const char * s2); 
#endif
