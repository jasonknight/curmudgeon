%{
    #include "haml_lexer.h"
    #include <math.h>
    #include <stdlib.h>
    #include <stdio.h>
%}
%pointer
%%  
[ \t]+                          { set_whitespace(yytext); }    
[\r\n\f]+                       { /*printf("NLS: %d\n",strlen(yytext));*/ }    
[%\.#][a-zA-Z\-_]+              { create_tag_node(yytext); }    
\{.+\}                          { /*printf("Options: %s\n",yytext);*/ }    
=.+$                            { /*printf("Directive: %s\n",yytext);*/ }  
[^ \r\n\f\t\{#%\.]+.+$          { /*printf("Text: %s\n",yytext);*/ }  
.                               { /*printf("unknown char %s\n",yytext);*/}
%%

void yyerror(const char *str) {
    fprintf(stderr,"error: %s\n",str);
}
 
int yywrap() {
    return 1;
}

int main(void) {
    
    printf("Creating r00t node\n");
    current_whitespace_length = 0;
    root_node = init_haml_node("_r00t_", -2);
    last_created_node = root_node;
    printf("Created r00t node at %X\n", root_node);
    
    FILE *myfile = fopen("../examples/template.haml", "r");
    if (!myfile) {
        printf("Can't open file\n");
        return -1;
    }
    yyin = myfile;
    yylex();
    fclose(myfile);
    

    
    print_tree(root_node);
}

haml_node_t * init_haml_node(char * tag_name, int ws) {
    //printf("  = called init_haml_node(%s)\n", tag_name);
    haml_node_t * node = malloc(sizeof(haml_node_t));
    node->children = malloc(sizeof(haml_node_t) * 5);
    node->tag_name = strdup(tag_name);
    node->type = 0;
    node->max_children = 5;
    node->child_count = 0;
    node->ws = ws;
    return node;
}

int append_new_haml_node(haml_node_t * parent, char * tag_name) {
    //printf("  = called append_new_haml_node(%X, %s)\n", parent, tag_name);
    //printf("    initializing new child node %s\n", tag_name);
    haml_node_t * new_child = init_haml_node(tag_name, current_whitespace_length);
    //printf("    initialized new child node %s\n", tag_name); 
    if (parent->child_count == parent->max_children - 1) {
        parent->max_children = parent->max_children + ceil((parent->max_children * 0.33)); // grow by 33%
        parent->children = realloc(parent->children, parent->max_children * sizeof(char *));
    }
    parent->children[parent->child_count++] = new_child;
    new_child->parent = parent;
    last_created_node = new_child;
    return 0;
}

int create_tag_node(char * haml_string) {
    //printf("  = called create_tag_node(%s)\n", haml_string);
    char * first_char = malloc(sizeof(char));
    
    strncpy(first_char, haml_string, 1);
    //printf("TAG: %s\n", haml_string);

    if (strcmp(first_char, "%") == 0) {
        printf("\n\nattempting to find parent for %s, ws=%i\n", haml_string, current_whitespace_length);
        haml_node_t * parent = find_parent(last_created_node);
        printf("parent of %s is %s\n", haml_string, parent->tag_name);
        //printf("Creating node %s for parent %X, children address %X\n", haml_string, parent, parent->children);
        append_new_haml_node(parent, haml_string);
        //printf("tag %s is not yet supported\n", haml_string);
        
    }
    return 0;
}

int set_whitespace(char * whitespace_string) {
    //printf("--> inside setting_whitespace\n");
    //printf("Setting whitespace count %i for node %s\n",strlen(yytext), last_created_node->tag_name);
    current_whitespace_length = strlen(yytext);
    return 0;
}

haml_node_t * find_parent(haml_node_t * current_node) {
    printf("Finding parent for %s address %X, ws=%i\n", current_node->tag_name, current_node, current_whitespace_length);
    if (current_node->parent == 0) {
        printf("No parent for _r00t_ node. Returning r00t node instead.\n");
        return current_node;
    }
    //haml_node_t * parent = current_node->parent;
    if (current_node->ws > current_whitespace_length - 2) {
        printf("    recursion\n");
        find_parent(current_node->parent);
    } else {
        return current_node;
    }
}

int print_tree(haml_node_t *current_node) {
    int i;
    haml_node_t *n, *c;
    n = current_node;
    char * spaces = malloc(sizeof(char) * current_node->ws);
    
    for (i=0; i<current_node->ws; i++) {
        spaces[i] = 0x20;
    }
    
    //printf("%s", spaces);
    printf("%s<Node tag_name: '%s', type: %i, ws: %i, child_count: %i, max_children: %i, text_contents: '%s' children: %X, parent: %X>\n", spaces, n->tag_name, n->type, n->ws, n->child_count, n->max_children, n->text_contents, n->children, n->parent);
    for (i=0; i<(n->child_count); i++) {
        c = n->children[i];
        print_tree(c);
    }
}


//haml_node_t * parse_haml_template(char * name) {}
//char * haml_node_to_html(haml_node_t * root) {}
