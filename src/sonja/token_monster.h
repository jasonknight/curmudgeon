#ifndef TOKEN_MONSTER_H
#define TOKEN_MONSTER_H
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
typedef struct token_monster_s token_monster_t;
typedef struct token_monster_dictionary_s token_monster_dictionary_t;
typedef int (*token_monster_callback_t)(token_monster_dictionary_t * dict, char * stream, int * head,token_monster_t * token);
typedef void (*token_monster_escape_callback_t)(
        token_monster_dictionary_t * dict,
        char * stream, 
        int * head,
        int *buffer_index, 
        char * buffer
);
struct token_monster_s {
    char        *text;
    char        started_with;
    int         start_column;
    char        ended_with;
    int         end_column;
    int         length;
    int         line_number;
    char        terminated_by;
    short       tag; // something you can tag the token with
    token_monster_t * previous;
    token_monster_t * next;
};
struct token_monster_dictionary_s {
    char        *name;
    char        *alphabet;
    char        *starts_with;
    char        *ends_with;
    char        escape_character;
    char        terminator;
    short       tigger;
    int         limit; // i.e. we could match a lot of character, so don't be greedy, match only
                       // what is necessary and move on
    int         min;
    token_monster_callback_t validator;
    token_monster_escape_callback_t escape_callback;
};  
token_monster_dictionary_t *    token_monster_create_dictionary(char * name);
void                            token_monster_debug_dictionary(token_monster_dictionary_t * dict);
int                             token_monster_dictionary_is_valid(token_monster_dictionary_t * dict);
token_monster_t *               token_monster_create_token();
token_monster_t *               token_monster_parse_string(char * stream, token_monster_dictionary_t * dicts[]);
token_monster_t *               token_monster_subparse_string(char * stream, int * cpos,token_monster_dictionary_t * cdict,token_monster_t * root);
token_monster_t *               token_monster_parse_file(char * filename, token_monster_dictionary_t * dicts[]);
int                             token_monster_dict_is_interested(token_monster_dictionary_t * dict, char c);
int                             token_monster_dict_matches(token_monster_dictionary_t * dict, char c);
void                            token_monster_debug_token(token_monster_t * token);


/*
 * Now we get into code that helps you to create a parser. This is a simple LL(n) style parser. Though
 * theoretically you could alter it in almost any way imaginable.
 *
 * With a parse, you have this basic idea of Nodes that have children. You go along a string
 * of tokens from left to right (A doubly linked list etc), and considering the "tag" of the
 * token, and perhaps the tag of the preceding, or following token, you make some kind of
 * decision about where it belongs in the "tree".
 *
 * So what we will do is this: 
 *
 * Given a set of tags and callbacks, loop over all tokens found and if a token is found
 * with an "interesting" tag, pass it and a "current" node to the callback.
 * */
typedef struct peter_parser_s peter_parser_t;
typedef struct peter_parser_node_s peter_parser_node_t;
/** 
 * @brief the parser struct, it is not necessary to give all rules to the parser,
 * you could in theory give a single rule that starts a state machine and takes over
 * the entire parsing process.
 * */
struct peter_parser_s {
    short * tags; // tags we are interested in, they should match the corresponding rule
                   // in the rules list, 0 -> 0, 1 -> 1 etc 
    int (**rules)(peter_parser_t *,token_monster_t *,peter_parser_node_t **); // the index of the tags should match the index of the rules 
    int count; // will be inialized to -1
    int capacity;
};
struct peter_parser_node_s {
    token_monster_t * token; // we already have a lot of info here.
    int     count;
    int     capacity;
    peter_parser_node_t ** children;
    peter_parser_node_t * parent;
};
peter_parser_t *        token_monster_create_parser(int num_rules);
peter_parser_node_t *   token_monster_create_node();
char *    token_monster_printable(char c);
void printucz(unsigned char * buf,int len) {
   int i = 0;
   while (i <= len) {
    printf("%u ",buf[i]);
    i++;
   }
}
#endif
