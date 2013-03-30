#include "tokenmonster.h"
/**
 * @brief Create a blank dictionary, initializing members 
 *
 * @param[in] name
 * @return a dictionary
 * */
token_monster_dictionary_t * token_monster_create_dictionary(char * name) {
    token_monster_dictionary_t * dict = malloc(sizeof(token_monster_dictionary_t));
    dict->name = name;
    dict->alphabet = "";
    dict->starts_with = "";
    dict->ends_with = "";
    dict->escape_character = '\0';
    dict->tigger = 1;
    dict->validator = NULL;
    dict->escape_callback = NULL;
    dict->tag = 0;
    return dict;
}
void token_monster_debug_dictionary(token_monster_dictionary_t * dict) {
    printf("<TokenMonsterDict tag: [%d] name: %s, alph: [%s], sw: %s, ew: %s, esc: %s tig: %d>\n",
            dict->tag,
            dict->name,
            dict->alphabet,
            dict->starts_with,
            dict->ends_with,
            token_monster_printable(dict->escape_character),
            dict->tigger);
}
/**
 * @brief used internally to validate a dictionay*/
int token_monster_dictionary_is_valid(token_monster_dictionary_t * dict) {
    if (dict->alphabet) {
        return 1;
    } else {
        return 0;
    }
}

token_monster_t * token_monster_parse_file(
    char * filename, 
    token_monster_dictionary_t * dicts[],
    char * cmnt_start, 
    char * cmnt_end
) {
    char * file_contents;
    FILE * fp;
    struct stat info;
    int bytes;
    token_monster_t * root;
    fp = fopen(filename,"r");
    if ( ! fp ) {
        printf("TokenMonster: Could not open file %s\n",filename);
    }
    fstat(fileno(fp), &info);
    bytes = sizeof(char) * (info.st_size + 1);
    file_contents = malloc(bytes);
    if ( ! file_contents ) {
        printf("could not allocate file_contents for %s\n",filename);
    }
    fread(file_contents, sizeof(char),info.st_size, fp);
    fclose(fp);
    root = token_monster_parse_string(file_contents,dicts,cmnt_start,cmnt_end);
    return root;
}
/**
 * @brief takes a char * stream and a NULL terminated list of dicts and returns a doubly linked list of tokens
 * */
token_monster_t * token_monster_parse_string(
    char * stream, 
    token_monster_dictionary_t * dicts[],
    char * cmnt_start,
    char * cmnt_end
) {
    char c; // the current character
    unsigned char first,second,third;
    int cpos = 0; // the current position in the string
    int saved_cpos = 0; //in case subparse returns null
    int dpos;     // which dictionary are we looking at?
    token_monster_dictionary_t * cdict = NULL;
    short string_is_utf8 = 0;
    token_monster_t * root = token_monster_create_token();
    root->text = "_r00t_";
    token_monster_t * last_token = root;
    token_monster_t * token = token_monster_create_token();
    first = stream[0];
    second = stream[1];
    third = stream[2];
    if (first == 0xef && second == 0xbb && third == 0xbf) {
        /* This means the string was prolly read from a file */
        if(g_token_monster_dbg_level == 1)
            printf("Utf8 BOM\n");
        cpos = 3;
        string_is_utf8 = 1; 
    } 
    get_next_char:
    while ( (c = stream[cpos]) ) {
        if(g_token_monster_dbg_level == 1)
            printf("[%d, %x, %s]\n",
                    cpos,
                    c,
                    token_monster_printable(c)
            );
        if (cmnt_start && strlen(cmnt_start) > 0) {
            // how many chars will we need to look ahead by?
            int cmnt_len = strlen(cmnt_start);
            int cmnt_i = 0;
            char tmp_c;
            for (cmnt_i = 0; cmnt_i < cmnt_len; cmnt_i++) {
                tmp_c = stream[cpos + cmnt_i];
                if (tmp_c != cmnt_start[cmnt_i]) {
                    goto no_comment; // this isn't a comment
                } else {
                    if(g_token_monster_dbg_level == 1)
                        printf("\t%d %c cmnt %s\n",cpos,tmp_c,cmnt_start);
                }
            }
            cmnt_len = strlen(cmnt_end);
            consume_comment:
            if ( ! cmnt_end ) {
                // this is a single line comment system,
                // we need to consume to the end of line
                while ( (stream[++cpos] && strchr("\r\n",stream[cpos]) == NULL) );
                goto no_comment;
            } else {
                while ( (strchr(cmnt_end,stream[++cpos]) == NULL) && stream[cpos] ) {
                    if(g_token_monster_dbg_level == 1)
                        printf("\t%d %c !cmnt_end %s\n",cpos,stream[cpos],cmnt_end); 
                }
            }
            // okay, we found a char, so let's make sure
            for (cmnt_i = 0; cmnt_i < cmnt_len; cmnt_i++) {
                tmp_c = stream[cpos + cmnt_i];
                if (tmp_c != cmnt_end[cmnt_i]) {
                    if(g_token_monster_dbg_level == 1)
                        printf("\t%d more comment. %c %s\n",cpos,tmp_c,cmnt_end);
                    goto consume_comment;
                } else {
                    if(g_token_monster_dbg_level == 1)
                        printf("\t%d %c cmnt_end %s\n",cpos,tmp_c,cmnt_end);
                }
            }
            cpos += cmnt_i;
            goto get_next_char;
        } // End comment parsing code
        no_comment:
        // Now we have the character, let's see if any of the dictionaries
        // are interested.
        choose_dictionary:
        dpos = 0;
        cdict = NULL;
        while ( (cdict = dicts[dpos]) ) {
            if (token_monster_dict_is_interested(cdict,c)) {
                // cool, it's interested
                // Now we have a dictionary that is interested in the input
                if(g_token_monster_dbg_level == 1)
                    printf("\t%s int. %d %s \n",
                            cdict->name,
                            cpos,
                            token_monster_printable(c)
                    );
                // At this point, we need to subparse the string.
                saved_cpos = cpos;
                token = token_monster_subparse_string(stream,&cpos,cdict,last_token);
                if (!token) {
                    cpos = saved_cpos;
                    if(g_token_monster_dbg_level == 1)
                        printf("\t restore %d\n",cpos);
                } else {
                    token->previous = last_token;
                    last_token->next = token;
                    last_token = token;
                    goto inc_cpos_and_continue;
                }
            }
            dpos++;
        } // iterate over dictionaries to find interest
        inc_cpos_and_continue:
        cpos++; // Don't forget this, serious, your computer will crash. 
    }
    done_processing:
    return root;
}
token_monster_t * token_monster_subparse_string(
    char * stream, 
    int * original_cpos,
    token_monster_dictionary_t * cdict,
    token_monster_t * last_token
) { 
    int cpos = *original_cpos;
    char * buf = malloc(sizeof(char) * 4000);
    char c = stream[cpos];
    int saved_cpos = cpos;
    token_monster_t * token;
    buf[0] = c;
    buf[1] = '\0';
    if(g_token_monster_dbg_level == 1)
        printf("\t\t%s ctrl %d %s\n",
                cdict->name,
                cpos,
                token_monster_printable(c)
        );
    cpos++;
    // At this point, we know we are interested in the
    int bj = 1;
    while ( (c = stream[cpos]) ) {
      if (token_monster_dict_matches(cdict,c)) {
        if(g_token_monster_dbg_level == 1)
            printf("\t\t\t[%d, %x, %s]\n",cpos,c,token_monster_printable(c)); 
        buf[bj++] = c;
        if (cdict->terminator && cdict->terminator == c) {
            buf[bj] = '\0';
            cpos++; // because we caught this char specially
            goto lost_interest;
        }
      } else {
        lost_interest:
        if(g_token_monster_dbg_level == 1)
            printf("\t\t\t%d %s !int [%s]\n",
                    cpos,
                    cdict->name,
                    token_monster_printable(c)
            ); 
        // Okay, we've lost interest, but now we need to validate the token
        // The rules for validation are
        // 1: ends_with?
        if (strlen(cdict->ends_with) > 0 && ! strchr(cdict->ends_with,buf[bj-1])) {
            if(g_token_monster_dbg_level == 1)
                printf("\t\t\t%d %s ends_with [%s] %s fail.\n",
                        cpos,
                        cdict->name,
                        cdict->ends_with,
                        token_monster_printable(buf[bj-1])
                );
            return NULL;
        } else {
            if(g_token_monster_dbg_level == 1)
                printf("\t\t\t%d %s ends_with [%s] %s succ.\n",
                        cpos,
                        cdict->name,
                        cdict->ends_with,
                        token_monster_printable(c)
                );
        }
        token = token_monster_create_token();
        token->text = strdup(buf);
        token->start_column = saved_cpos;
        token->end_column = cpos;
        token->length = strlen(token->text);
        //token->line_number = line;
        token->tag = cdict->tag;
        token->terminated_by = c;
        if(g_token_monster_dbg_level == 1)
            printf("\t\t\t\t%s tagging %s with %d\n",cdict->name,token->text,cdict->tag);
        // Okay, ends with is okay, but the validator is the final arbitor
        if (cdict->validator != NULL) {
            if (cdict->validator(cdict,stream,&cpos,token) ==  1) {
                if(g_token_monster_dbg_level == 1)
                    printf("\t\t\t %s vdtor succ.\n",cdict->name);
            } else {
                if(g_token_monster_dbg_level == 1)
                    printf("\t\t\t %s vdtor failed.\n",cdict->name);
                free(token->text);
                free(token);
                return NULL;
            }
        } // otherwise, we don't bother because a validator wasn't set
        if(g_token_monster_dbg_level == 1)
            printf("\t\t\t Buffer is: %s\n",buf);
        
        last_token = token;
        cpos--;
        break;
      }
      cpos++;
    }
    *original_cpos = cpos; // put the current position into the caller's cpos
    if(g_token_monster_dbg_level == 1)
        printf("\t\t%s dne %d\n",cdict->name,cpos);
    return last_token;
}

/**
 * @brief helper function to convert a non-printing char into a printable char *
 * */
char * token_monster_printable(char c) {
    if (c == 0xa) {
        return "\\n";
    }
    if (c == 0x20) {
        return "\\s";
    }
    char * ret = malloc(sizeof(char) * 2);
    sprintf(ret,"%c",c);
    ret[1] = '\0';
    return ret;
}
/**
 * @brief initialize and new token, null'ing out values
 * */
token_monster_t * token_monster_create_token() {
    token_monster_t * token = malloc(sizeof(token_monster_t));
    token->next = NULL;
    token->previous = NULL;
    return token;
}
/**
 * @brief Decides whether or not a dictionary is interested in this input char
 *
 * Dictionaries work on simple rules, like starts_with some class of characters.
 * It's not really like a regular expression, for our purposes, a Regex is too
 * complicated. 
 * */
int token_monster_dict_is_interested(token_monster_dictionary_t * dict, char c) {
    if ( strlen(dict->starts_with) > 0) {
        // The c must be in the begins_with set
        if (strchr(dict->starts_with,c)) {
            return 1 ;
        } else {
            return 0;
        }
    }
    // We don't have a begins with, so let's see if it is in the alpha set
    int is_in_set      = ( strchr(dict->alphabet,c) != NULL );
    if (c == dict->escape_character) {
        is_in_set = 1;
    }
    return ( is_in_set == dict->tigger );
}
/**
 * @brief decides if a dictionary is still interested in the input
 *
 * This is necessary because the rules for starting and continuing are
 * not the same. 
 * 
 * This method abstracts away the concept of a tiggerton/eeyoreton,
 * that is, whether or not we are looking for inclusion, or exclusion.
 *
 * In the end, we compare the strchr(s,c) == NULL to the dict->tigger
 * value. dict->tigger will be 0 if we are looking for exclusion,
 * or dict->tigger will be 1 if we are looking for inclusion.
 * */
int token_monster_dict_matches(token_monster_dictionary_t * dict, char c) {
    int is_in_set      = ( strchr(dict->alphabet,c) != NULL );
    if (c == dict->escape_character) {
        if (g_token_monster_dbg_level == 1)
            printf("setting is_in_set to 1 because of escape char");
        is_in_set = 1;
    }
    return ( is_in_set == dict->tigger );
}
void token_monster_debug_token(token_monster_t * token) {
    printf("\t<Token text: [%s] tag: [%d] terminated_by: [%s]>\n",
            token->text,
            token->tag,
            token_monster_printable(token->terminated_by)
    );

}
/**
 * Parser Functions
 * */
/**
 * @brief creates and initializes a parser to which we can add rules
 * @param[in] number of rules to prealloc*/
peter_parser_t * token_monster_create_parser() {
    peter_parser_t * parser = malloc(sizeof(peter_parser_t));
    parser->count = -1;
    parser->capacity = 0;
    parser->memsize = 0;
    return parser;
}
int token_monster_add_rule(
    peter_parser_t * peter,
    int tag,
    int (*rule)(peter_parser_t *, token_monster_t *,peter_parser_node_t **)
) {
    int int_bytes,cb_bytes;

    if (peter->count == -1) {
        int_bytes = sizeof(int) * 5;
        cb_bytes = sizeof(void *) * 5;
        peter->rules = malloc(cb_bytes);
        if (!peter->rules)
            return -1;
        peter->tags = malloc(int_bytes);
        if (!peter->tags)
            return -2;
        peter->capacity = 5;
        peter->memsize = cb_bytes + int_bytes;
    } else if (peter->count == peter->capacity - 1) {
        peter->capacity += 5;
        int_bytes = sizeof(int) * peter->capacity;
        cb_bytes = sizeof(void *) * peter->capacity;
        peter->rules = realloc(peter->rules,cb_bytes);
        if (!peter->rules)
            return -1;
        peter->tags = realloc(peter->tags,int_bytes);
        if (!peter->tags)
            return -2;
        peter->memsize = int_bytes + cb_bytes;
    }
    peter->tags[++peter->count] = tag;
    peter->rules[peter->count]   = rule;
    return 0;
}
int  token_monster_rule_for(
    peter_parser_t * peter,
    int tag,token_monster_t * token,
    peter_parser_node_t **node
) {
    int i;
    for (i = 0; i <= peter->count; i++) {
        if ( peter->tags[i] == tag && ! peter->rules[i] == NULL) {
            return (*peter->rules[i])(peter,token,node);
        }
    }
    return -2;
}
peter_parser_node_t * token_monster_create_node() {
    peter_parser_node_t * node = malloc(sizeof(peter_parser_node_t));
    node->count = -1; // so we cna just ++ and get going
    node->capacity = 5;
    node->children = malloc(sizeof(peter_parser_node_t *) * node->capacity);
    int i;
    for (i = 0; i < node->capacity; node->children[i++] = NULL);
    node->parent = NULL;
    return node;
}
