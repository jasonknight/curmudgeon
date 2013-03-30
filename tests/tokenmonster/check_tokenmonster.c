#include "tokenmonster.h"
#include <check.h>
#include <stdlib.h>
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
START_TEST(test_json_lexing) {
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
    token_monster_t * token = root->next->next;
    ck_assert_str_eq(token->text,"test");
    token = token->next->next; // skip the colon
    ck_assert_str_eq(token->text,"true");
    ck_assert_int_eq(token->tag,JSONBoolean);
    token = token->next->next;
    ck_assert_str_eq(token->text,"test2");
    token = token->next->next;
    ck_assert_int_eq(token->tag,JSONBoolean);
    token = token->next->next;
    ck_assert_str_eq(token->text,"test3");
    token = token->next->next;
    ck_assert_int_eq(token->tag,JSONNull);
    token = token->next->next;
    ck_assert_str_eq(token->text,"test4");
    token = token->next->next;
    ck_assert_int_eq(token->tag,JSONNumber);
    token = token->next->next->next->next;
    ck_assert_int_eq(token->tag,JSONString);

   // do {
   //    token_monster_debug_token(token);
   //    fflush(stdout);
   // } while( (token = token->next) );    
}
END_TEST

START_TEST(basic_test) {
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
    fail_unless( (strcmp(root->next->text,"test") == 0), "Expected test as token value");
    dicts[0]->escape_character = '\\';
    dicts[0]->escape_callback = test_escape_callback;
    teststring = "char with i\\e escape";
    token_monster_debug_dictionary(dicts[0]);
    root = token_monster_parse_string(teststring,dicts,NULL,NULL);
    fail_unless( (root != NULL), "root is null!");
    fail_unless( (strcmp(root->text,"_r00t_") == 0 ), "root misnamed");
    fail_unless( (root->next != NULL), "No second token");
    ck_assert_str_eq(root->next->next->text,"with");
    fail_unless( (strcmp(root->next->next->text,"with") == 0), "Expected with as token value");
}
END_TEST
START_TEST(basic_parser) {
    peter_parser_t * peter = token_monster_create_parser();
    ck_assert_int_eq(peter->count,-1);
    ck_assert_int_eq(peter->capacity,0);
    ck_assert_int_eq(peter->memsize,0);
}
END_TEST
peter_parser_node_t * rule_callback(
    peter_parser_t * peter,
    token_monster_t * token, 
    peter_parser_node_t * node
) {
    
    return node;
}
START_TEST(parser_add_rule) {
    peter_parser_t * peter = token_monster_create_parser();
    token_monster_add_rule(peter,JSONObject,rule_callback);
    ck_assert_int_eq(peter->count,0);
    ck_assert_int_eq(peter->capacity,5);
    ck_assert_int_eq(peter->tags[0],JSONObject);
}
END_TEST
START_TEST(parser_add_many_rules) {
    peter_parser_t * peter = token_monster_create_parser();
    int i;
    for (i = 0; i < 8; i++) {
        token_monster_add_rule(peter,i,rule_callback);
    }
    int ib,cb;
    ib = sizeof(int) * 10;
    cb = sizeof(void *) * 10;
    ck_assert_int_eq(peter->memsize,ib+cb);
    ck_assert_int_eq(peter->count,7);
    ck_assert_int_eq(peter->tags[7],i-1);
}
END_TEST
START_TEST(parser_execute_rule) {
    peter_parser_t * peter = token_monster_create_parser();
    token_monster_add_rule(peter,JSONObject,rule_callback);
    token_monster_t * token = token_monster_create_token();
    peter_parser_node_t * node = token_monster_create_node();
    ck_assert_int_eq(token_monster_has_rule_for(peter,JSONObject),1);
    peter_parser_node_t * rc = token_monster_rule_for(peter,JSONObject,token,node);
    fail_unless(rc != NULL,"Rule not found, when it should have been");
    ck_assert_int_eq(token_monster_has_rule_for(peter,99),0);
    rc = token_monster_rule_for(peter,99,token,node);
    fail_unless(rc == NULL);
}
END_TEST



Suite * create_token_monster_suite(void) {
    Suite * s       = suite_create("Token Monster");
    TCase * tc      = tcase_create("Basic Parsing");
    tcase_add_test(tc,basic_test);
    tcase_add_test(tc,test_json_lexing);
    tcase_add_test(tc,basic_parser);
    tcase_add_test(tc,parser_add_rule);
    tcase_add_test(tc,parser_add_many_rules);
    tcase_add_test(tc,parser_execute_rule);
    suite_add_tcase(s,tc);
    return s;
}
int main() {
    int number_failed;
    g_token_monster_dbg_level =  0;
    Suite * s = create_token_monster_suite();
    SRunner *sr = srunner_create(s);
    srunner_run_all(sr,CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
    //test();
    //printf("Now for something difficult\n"); 
    //test_string_parsing();
    //printf("Testing complex JSON Parsing\n");
    //test_json_lexing();
    //printf("\nAll tests passing\n");

    return 0;
}

