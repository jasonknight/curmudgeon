#include "token_monster.h"
enum Tags {
    JSONString,
    JSONNumber,
    JSONObject,
    JSONArray,
    JSONBoolean,
    JSONNull
};
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

    token_monster_t * root = token_monster_parse_file("./string-test.txt",dicts,NULL,NULL);
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
    if (strcasecmp(token->text,"null") == 0) {
        token->tag = JSONNull;
    }
    return 1; 
}
int test_object_validator(token_monster_dictionary_t * dict, char * stream, int * head, token_monster_t * token) {
    if (strcmp("{",token->text) == 0) {
        printf("\t\t - tagging as object.\n");
        token->tag = JSONObject;
    }
    if (strchr("[",token->text[0])) {
        printf("\t\t - tagging as array.\n");
        token->tag = JSONArray;
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
    d->validator = test_object_validator;
    dicts[0] = d;

    d = token_monster_create_dictionary("DQ");
    d->alphabet = "";
    d->tigger = 0; // i.e. we accept everything
    d->starts_with = "\"";
    d->escape_character = '\\'; // we'll need to preserve this
    d->terminator = '"'; // we don't need an ends_with rule, because terminator ensures this.
    d->tag = JSONString;
    token_monster_debug_dictionary(d);
    dicts[1] = d;
    
    d = token_monster_create_dictionary("SQ");
    d->alphabet = "";
    d->tigger = 0; // i.e. we accept everything
    d->starts_with = "'";
    d->terminator = '\''; // we don't need an ends_with rule, because terminator ensures this.
    d->tag = JSONString;
    token_monster_debug_dictionary(d);
    dicts[2] = d;
    
    d = token_monster_create_dictionary("N");
    d->alphabet = "+-0123456789.eEx";
    d->starts_with = "+-0123456789";
    d->tag = JSONNumber;
    token_monster_debug_dictionary(d);
    dicts[3] = d;
    
    d = token_monster_create_dictionary("B");
    d->alphabet = "tTrRuUeEfFaAlLsSnNuU";
    d->starts_with = "tTfFnN";
    d->ends_with = "eELl";
    d->limit = 5;
    d->validator = test_bool_validator;
    d->tag = JSONBoolean;
    token_monster_debug_dictionary(d);
    dicts[4] = d;

    d = token_monster_create_dictionary("Id");
    d->alphabet = "\n\r :,[]{}";
    d->tigger = 0;
    d->starts_with = "abcdefghijklmnopqrstuvwzyz_";
    token_monster_debug_dictionary(d);
    dicts[5] = d;
    
    d = token_monster_create_dictionary("Pu");
    d->alphabet = ",:";
    d->limit = 1;
    token_monster_debug_dictionary(d);
    dicts[6] = d;

    //g_token_monster_dbg_level = 1;
    token_monster_t * root = token_monster_parse_file("./json-test.js",dicts,"/*","*/");
    token_monster_t * token = root;

    do {
       token_monster_debug_token(token);
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
    token_monster_t * root = token_monster_parse_string(teststring,dicts,NULL,NULL);
    dicts[0]->escape_character = '\\';
    dicts[0]->escape_callback = test_escape_callback;
    teststring = "char with i\\e escape";
    token_monster_debug_dictionary(dicts[0]);
    root = token_monster_parse_string(teststring,dicts,NULL,NULL);
    token_monster_t * token = root;

        do {
            token_monster_debug_token(token);
        } while( (token = token->next) );

}
int main() {
    g_token_monster_dbg_level =  0;
    //test();
    //printf("Now for something difficult\n"); 
    //test_string_parsing();
    //printf("Testing complex JSON Parsing\n");
    test_json_lexing();
    printf("\nAll tests passing\n");

    return 0;
}

