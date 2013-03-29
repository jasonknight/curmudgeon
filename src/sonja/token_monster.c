#include "token_monster.h"
int test_validator(token_monster_dictionary_t * dict,char * stream, int * head, token_monster_t * token) {
    int th = *head;
   // printf("Hello from the validator next char is: %c!\n",stream[th + 1]);
    return 1; 
}
int parser_test_callback(peter_parser_t * parser, token_monster_t * token, peter_parser_node_t ** current_node) {
    printf("Hello from the parser callback\n");
    return 0;
}
void test_escape_callback(token_monster_dictionary_t * dict,char * stream, int * head, int * buffer_index, char * buffer) {
     buffer[*buffer_index] = '\\'; // allows us to maintain the escape sequence.
     buffer[++(*buffer_index)] = stream[++(*head)];
    //printf("hello from the escape callback");
}
void test_string_parsing() {
    token_monster_dictionary_t * dicts[5];
    int i;
    for(i = -1; i < 5; dicts[++i] = NULL); // always remember to null!

    token_monster_dictionary_t * dict = token_monster_create_dictionary("StringDoubleQuoted");

    dict->alphabet = "";
    dict->tigger = 0;
    dict->starts_with = "\"";
    dict->ends_with = "\"";
    dict->terminator = '"';
    
    dicts[0] = dict;

    token_monster_dictionary_t * dict2 = token_monster_create_dictionary("Ident");

    dict2->alphabet = "abcdefghijklmnopqrstuvwxyz";
    dict2->tigger = 1;

    dicts[1] = dict2;

    token_monster_t * root = token_monster_parse_file("./string-test.txt",dicts);
    token_monster_t * token = root;
    
    // Create the parser
    peter_parser_t * parser = token_monster_create_parser(2);
    parser->tags[0] = 0;
    parser->rules[0] = parser_test_callback;
    parser->tags[1] = 1;
    parser->rules[1] = parser_test_callback;
    peter_parser_node_t * current_node = token_monster_create_node();

    do {
        token_monster_debug_token(token);
        // This looks all crazy, but it's just an array of 
        // function pointers.
        //(*parser->rules[0])(parser, token, &current_node); 

    } while( (token = token->next) );
}
int test_json_tagger(token_monster_dictionary_t * dict, char * stream, int * head, int * buffer_index, char * buffer) {
   
    return 1; 
}
int test_bool_validator(token_monster_dictionary_t * dict,char * stream, int * head, token_monster_t * token) {
    if (strlen(token->text) < 4) {
        return 0;
    }
    return 1; 
}
void test_json_lexing() {
    int i;
    // We will need dictionaries for:
    // 1 Braces {}[]
    // 2 Single quoted strings # Need to preserve the escape!
    // 3 Double quoted strings # we don't preserve the escape
    // 4 numbers
    // 5 true,false,null
    // 6 Ident literals
    token_monster_dictionary_t * dicts[8];
    for(i = 0; i < 8; dicts[i++] = NULL); // always remember to null!
    token_monster_dictionary_t * d = token_monster_create_dictionary("Braces");
    d->alphabet = "{[]}";
    d->limit = 1;
    dicts[0] = d;

    d = token_monster_create_dictionary("DQ");
    d->alphabet = "";
    d->tigger = 0; // i.e. we accept everything
    d->starts_with = "\"";
    d->escape_character = '\\'; // we'll need to preserve this
    d->terminator = '"'; // we don't need an ends_with rule, because terminator ensures this.
    dicts[1] = d;
    
    d = token_monster_create_dictionary("SQ");
    d->alphabet = "";
    d->tigger = 0; // i.e. we accept everything
    d->starts_with = "'";
    d->terminator = '\''; // we don't need an ends_with rule, because terminator ensures this.
    dicts[2] = d;
    
    d = token_monster_create_dictionary("N");
    d->alphabet = "+-0123456789.eEx";
    d->starts_with = "+-0123456789";
    dicts[3] = d;
    
    d = token_monster_create_dictionary("B");
    d->alphabet = "tTrRuUeEfFaAlLsSnNuU";
    d->starts_with = "tTfFnN";
    d->ends_with = "eELl";
    d->limit = 5;
    d->validator = test_bool_validator;
    dicts[4] = d;

    d = token_monster_create_dictionary("Id");
    d->alphabet = "\n\r :,[]{}";
    d->tigger = 0;
    d->starts_with = "abcdefghijklmnopqrstuvwzyz_";
    dicts[5] = d;
    
    d = token_monster_create_dictionary("Pu");
    d->alphabet = ",:";
    d->limit = 1;
    dicts[6] = d;

    token_monster_t * root = token_monster_parse_file("./json-test.js",dicts);
    token_monster_t * token = root;

    do {
       // token_monster_debug_token(token);
    } while( (token = token->next) );    
}

void test() {
    token_monster_dictionary_t *dict = token_monster_create_dictionary("MyDict"); 
    token_monster_debug_dictionary(dict);
    assert( token_monster_dictionary_is_valid(dict) == 1);
    
    token_monster_dictionary_t * dicts[5];
    int i;
    for(i = -1; i < 5; dicts[++i] = NULL); // always remember to null!
    dicts[0] = token_monster_create_dictionary("Testing");
    dicts[0]->alphabet = "abcdefghijklmnopqrstuvwxyz";
    dicts[0]->validator = test_validator;
    char * teststring = "test: string";
    token_monster_t * root = token_monster_parse_string(teststring,dicts);
    dicts[0]->escape_character = '\\';
    dicts[0]->escape_callback = test_escape_callback;
    teststring = "char with i\\e escape";
    token_monster_debug_dictionary(dicts[0]);
    root = token_monster_parse_string(teststring,dicts);
    token_monster_t * token = root;

        do {
            token_monster_debug_token(token);
        } while( (token = token->next) );

}
int main() {
    //test();
    //printf("Now for something difficult\n"); 
    //test_string_parsing();
    test_json_lexing();
    printf("\nAll tests passing\n");

    return 0;
}
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
    return dict;
}
void token_monster_debug_dictionary(token_monster_dictionary_t * dict) {
    printf("<TokenMonsterDict name: %s, alph: [%s], sw: %s, ew: %s, esc: %c tig: %d>\n",
            dict->name,
            dict->alphabet,
            dict->starts_with,
            dict->ends_with,
            dict->escape_character,
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

token_monster_t * token_monster_parse_file(char * filename, token_monster_dictionary_t * dicts[]) {
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
    root = token_monster_parse_string(file_contents,dicts);
    return root;
}
/**
 * @brief takes a char * stream and a NULL terminated list of dicts and returns a doubly linked list of tokens
 * */
token_monster_t * token_monster_parse_string(char * stream, token_monster_dictionary_t * dicts[]) {
    char c; // the current character
    unsigned char first,second,third;
    int cpos = 0; // the current position in the string
    int saved_cpos = 0; //in case subparse returns null
    int dpos;     // which dictionary are we looking at?
    token_monster_dictionary_t * cdict = NULL;
    short string_is_utf8 = 0;
    token_monster_t * root = token_monster_create_token();
    token_monster_t * tmp_root = token_monster_create_token();
    first = stream[0];
    second = stream[1];
    third = stream[2];
    if (first == 0xef && second == 0xbb && third == 0xbf) {
        /* This means the string was prolly read from a file */
        printf("Utf8 BOM\n");
        cpos = 3;
        string_is_utf8 = 1; 
    } 
    while ( (c = stream[cpos]) ) {
        printf("[%x,%s]\n",c,token_monster_printable(c));
        // Now we have the character, let's see if any of the dictionaries
        // are interested.
        dpos = 0;
        cdict = NULL;
        while ( (cdict = dicts[dpos]) ) {
            if (token_monster_dict_is_interested(cdict,c)) {
                // cool, it's interested
                // Now we have a dictionary that is interested in the input
                printf("\t%s int. %d %s \n",cdict->name,cpos,token_monster_printable(c));
                // At this point, we need to subparse the string.
                saved_cpos = cpos;
                tmp_root = token_monster_subparse_string(stream,&cpos,cdict,root);
                if (!tmp_root) {
                    cpos = saved_cpos;
                    printf("\t restore %d\n",cpos);
                }
            }
            dpos++;
        } // iterate over dictionaries to find interest
        
        cpos++; // Don't forget this, serious, your computer will crash. 
    }
    return root;
}
token_monster_t * token_monster_subparse_string(
    char * stream, 
    int * original_cpos,
    token_monster_dictionary_t * cdict,
    token_monster_t * root 
) { 
    int cpos = *original_cpos;
    char * buf = malloc(sizeof(unsigned char) * 4000);
    char c = stream[cpos];
    token_monster_t * token;
    buf[0] = c;
    buf[1] = '\0';
    printf("\t\t%s ctrl %d %s\n",cdict->name,cpos,token_monster_printable(c));
    cpos++;
    // At this point, we know we are interested in the
    int bj = 1;
    while ( (c = stream[cpos++]) ) {
      if (token_monster_dict_matches(cdict,c)) {
        printf("\t\t\t[%x,%s]\n",c,token_monster_printable(c)); 
        buf[bj++] = c;
        if (cdict->terminator && cdict->terminator == c) {
            buf[bj] = '\0';
            goto lost_interest;
        }
      } else {
        lost_interest:
        printf("\t\t\t %s !int [%s]\n",cdict->name,token_monster_printable(c)); 
        // Okay, we've lost interest, but now we need to validate the token
        // The rules for validation are
        // 1: ends_with?
        if (strlen(cdict->ends_with) > 0 && ! strchr(cdict->ends_with,(char)c)) {
            printf("\t\t\t %s ends_with [%s] %s fail.\n",cdict->name,cdict->ends_with,token_monster_printable(c));
            return NULL;
        } else {
            printf("\t\t\t %s ends_with [%s] %s succ.\n",cdict->name,cdict->ends_with,token_monster_printable(c));
        }
        // Okay, ends with is okay, but the validator is the final arbitor
        if (cdict->validator != NULL) {
            if (cdict->validator(cdict,stream,&cpos,root) ==  1) {
                printf("\t\t\t %s vdtor succ.\n",cdict->name);
            } else {
              printf("\t\t\t %s vdtor failed.\n",cdict->name);
              return NULL;
            }
        } // otherwise, we don't bother
        printf("\t\t\t Buffer is: %s\n",buf);
        token = malloc(sizeof(token_monster_t));
        return NULL;
        break;
      }
    }
    *original_cpos = cpos;
    printf("\t\t%s dne %d\n",cdict->name,cpos);
    return root;
}
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
//token_monster_t * token_monster_parse_string(char * stream, token_monster_dictionary_t * dicts[]) {
//    char              current_character; // the current char from the stream;
//    int               number_of_matches = 0; // How many successful matches we have
//    short             just_started      = 0; // did we just start the dictionary match?
//    int               length            = strlen(stream);
//    int               head              = 0; // our position in the stream
//                                             // a  b  c  d
//                                             // 0  1  2  3 etc
//    int               j                 = 0;
//    int               bj                = 0; // buffer iterator for sub parsing
//    int               saved_head        = 0;
//    int               line              = 0;
//    token_monster_dictionary_t * cdict = NULL;
//    token_monster_t *            root_token = token_monster_create_token();
//                                 root_token->previous = NULL;
//                                 root_token->next     = NULL;
//                                 root_token->text = "__r00t__";
//    token_monster_t *            last_token = root_token;
//    char * buffer = malloc(sizeof(char) * 4000);
//    // We need to go char by char
//    next_char:
//    while ( (current_character = stream[head]) ) {
//        printf("[%c]\n",current_character);
//        // Record the line endings
//        if (strchr("\n\r",current_character)) {
//            line++;
//        }
//        // now we need to know if any of the dictionaries are interested?
//       j = -1;
//       while ( (cdict = dicts[++j]) ) {
//           //printf("cdict is: %s ",cdict->name);
//           just_started = 1;  
//           if ( token_monster_dict_is_interested(cdict,current_character) ) {
//                //printf("%s is interested in '%c' ",cdict->name,current_character);
//                just_started = 0;
//                // This dict contains that char, so it's interested.
//                // now we need a buffer to put all characters in
//                // our buffer is 4kb, so if we go past 4k, we'll need to resize 
//                bj = 0;
//                buffer[bj] = current_character;
//                saved_head = head;
//                head++;
//                while ( (current_character = stream[head]) && token_monster_dict_matches(cdict,current_character) ) {
//                    printf(" --%c-- \n",current_character);
//                    bj++;
//                    if ( current_character != cdict->escape_character && current_character == cdict->terminator) {
//                        //printf("Terminator found %c\n", current_character);
//                        buffer[bj] = current_character;
//                        buffer[++bj] = '\0';
//                        break;
//                    }
//                    //printf(" -- %c %c -- \n",cdict->escape_character,current_character);
//                    if (current_character == cdict->escape_character) { 
//                        if (cdict->escape_callback) { 
//                            //printf("Escape Callback\n");
//                            cdict->escape_callback(cdict,stream,&head,&bj,buffer); 
//                            continue;
//                        } else {
//                            //printf("No escape callback");
//                            buffer[bj] = stream[++head];
//                            continue;
//                        }
//                    }
//                    buffer[bj] = current_character;
//                    number_of_matches++;
//                    head++;
//                    if (cdict->limit > 0 && number_of_matches >= cdict->limit) {
//                        printf("breaking because of limit %d",cdict->limit);
//                        break; 
//                    } 
//                } // end buffer concat loop
//                buffer[++bj] = '\0';
//                //printf("%s lost interest with %c buffer is %s",cdict->name,current_character,buffer);
//                // at this point, we've filled out buffer
//                // before we create our token, we need to check the ends
//                // with of the dictionary to disqualify matches
//                number_of_matches = 0; // reset num matches
//                if (strlen(cdict->ends_with) > 0) {
//                    //printf("ends_with '%s' %c\n",cdict->ends_with, buffer[bj]);
//                    if ( strchr(cdict->ends_with,buffer[bj]) == NULL ) { // bj++ didn't execute, 
//                                                                         // so it should still be the last char
//                        head = saved_head; //restore the reading head
//                        current_character = stream[head];
//                        //printf(" restored to '%c' ",stream[saved_head]);
//                        buffer[0] = '\0';
//                        continue; //next in the loop
//                    }
//                }
//                // so we need to create a new token
//                token_monster_t * token = token_monster_create_token();
//                
//                token->text = strdup(buffer);
//                // Now we need to validate the token, if there is a validator
//                if (cdict->validator && !cdict->validator(cdict,stream,&head,token)) {
//                    head = saved_head; 
//                    current_character = stream[head];
//                    buffer[0] = '\0';
//                    free(token);
//                    //printf(" vdtor restored to '%c' ",stream[saved_head]);
//                    continue;
//                }
//                last_token->next = token;
//                token->previous = last_token;
//                token->start_column = saved_head;
//                token->end_column = head;
//                token->length = strlen(token->text);
//                token->line_number = line;
//                token->terminated_by = stream[head];
//                //printf("%s success c is %c\n",cdict->name,current_character);
//                token_monster_debug_token(token);
//                last_token = token;
//                buffer[0] = '\0';
//                head++;
//                goto next_char;
//           }
//       } // while cdict = dicts
//       head++; // DON'T FORGET TO INCREMENT!
//    } // end main while current_char
//    return root_token;
//}
token_monster_t * token_monster_create_token() {
    token_monster_t * token = malloc(sizeof(token_monster_t));
    token->next = NULL;
    token->previous = NULL;
    return token;
}
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
int token_monster_dict_matches(token_monster_dictionary_t * dict, char c) {
    int is_in_set      = ( strchr(dict->alphabet,c) != NULL );
    if (c == dict->escape_character) {
        printf("setting is_in_set to 1 because of escape char");
        is_in_set = 1;
    }
    return ( is_in_set == dict->tigger );
}
void token_monster_debug_token(token_monster_t * token) {
    printf("\t<Token text: [%s] terminated_by: [%c]>\n",token->text,token->terminated_by);

}
/**
 * Parser Functions
 * */
/**
 * @brief creates and initializes a parser to which we can add rules
 * @param[in] number of rules to prealloc*/
peter_parser_t * token_monster_create_parser(int num_rules) {
    int tag_bytes = sizeof(short) * num_rules;
    int i;
    peter_parser_t * parser = malloc(sizeof(peter_parser_t));
    parser->tags = malloc(tag_bytes);
    parser->rules = malloc(sizeof(void *) * num_rules + 1);
    parser->count = 0;
    parser->capacity = num_rules;
    return parser;
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
