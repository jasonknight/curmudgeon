%{
    #include <math.h>
    #include <stdlib.h>
    #include <stdio.h>
    #include "sass.h"
%}
%pointer
%%  
[ \t]+                       { /*printf("WS%i\n",strlen(yytext)); */}
[\r\n\f]+                    { /*printf("NL: %d\n",strlen(yytext));*/ }
:.*                          { /*printf("ATTR         %s\n",yytext);*/ }
$.*                          { /*printf("VAR          %s\n",yytext);*/ }
@.*                          { /*printf("DIRECTIVE    %s\n",yytext);*/ }
[#\.].*                      { /*printf("ATTR_NODE    %s\n",yytext);*/ }
[^ \t\r\n\f\:\.#].*          { sass_create_tag_node(strdup(yytext)); }
.                            { /*printf("unknown char %s\n",yytext);*/}
%%

void yyerror(const char *str) {
    fprintf(stderr,"error: %s\n",str);
}
 
int yywrap() {
    return 1;
}


sass_node_t *sass_parse_file(char *name) {
printf("\n==========last is %X\n", last_created_node);
printf("\n==========lastt is %i\n", current_whitespace_length);
/*    FILE *myfile = fopen(name, "r");
    if (!myfile) {
        printf("Can't open file '%s'\n",name);
        return NULL;
    } else {
        printf("Opened file %s\n", name);
    }
    yyin = myfile;
    printf("calling yylex\n");
    yylex();
    printf("yylex called\n");
    fclose(myfile);
    return 0;*/
}

sass_node_t * sass_init_node(char *selector_name, int ws) {  
    sass_node_t *node = malloc(sizeof(sass_node_t));
    node->children  = malloc(sizeof(sass_node_t) * 5);
    node->attrs     = malloc(sizeof(sass_attr_t) * 5);
    node->selector_name = strdup(selector_name);
    node->max_children = 5;
    node->child_count = 0;
    node->max_attrs = 5;
    node->attr_count = 0;
    node->ws = ws;
    return node;
}

int sass_create_tag_node(char *sass_string) {
    printf("\nsass string is %s\n", sass_string);
    sass_node_t *parent;
    
    parent = sass_find_parent(last_created_node);
    printf("\nXXX%X\n", parent);
    /*
    last_created_node = sass_append_new_node(parent, sass_string);
    */
    return 0;
}

sass_node_t *sass_find_parent(sass_node_t *current_node) {
    printf("inside sass_find_parent current_node is %X", current_node);
    if (current_node->parent == 0) {
        printf("no parent");
        // this is the root node
        return current_node;
    }
/*    if (current_node->ws > current_whitespace_length - 2) {
        return sass_find_parent(current_node->parent);
    } else {
        return current_node;
    }*/
}

sass_node_t * sass_append_new_node(sass_node_t *parent, char *selector_name) {
    sass_node_t *new_child;
    
    new_child = sass_init_node(selector_name, current_whitespace_length);
    if (parent->child_count == parent->max_children - 1) {
        parent->max_children = parent->max_children + ceil((parent->max_children * 0.33)); // grow by 33%
        parent->children = realloc(parent->children, parent->max_children * sizeof(char *));
    }
    parent->children[parent->child_count++] = new_child;
    new_child->parent = parent;
    return new_child;
}


int print_tree(sass_node_t *current_node, int recursion_depth) {
    int i;
    sass_node_t *n, *c;
    n = current_node;
    char * spaces = malloc(sizeof(char) * (current_node->ws + 4) * recursion_depth);
    
    for (i=0; i<2*recursion_depth; i++) {
        spaces[i] = 0x20;
    }
    recursion_depth++;
    char * fmt = "%s  <Node#%X selector_name: '%s'>\n";
    printf(fmt, spaces, 
                n,
                n->selector_name);

    for (i=0; i<(n->child_count); i++) {
        c = n->children[i];
        print_tree(c, recursion_depth);
    }
}