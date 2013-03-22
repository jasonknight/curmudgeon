%{
    #include "haml_lexer.h"
    #include <math.h>
    #include <stdlib.h>
    #include <stdio.h>
%}
%pointer
%%  
[ \t]+                          { set_whitespace(yytext); }    
[\r\n\f]+                       { printf("NLS: %d\n",strlen(yytext)); }    
[%\.#][a-zA-Z\-_].+              { create_tag_node(yytext); }    
\{.+\}                          { printf("Options: %s\n",yytext); }    
=.+$                            { printf("Directive: %s\n",yytext); }  
[^ \r\n\f\t\{#%\.]+.+$          { create_text_node(yytext); }  
.                               { printf("unknown char %s\n",yytext);}
%%

void yyerror(const char *str) {
    fprintf(stderr,"error: %s\n",str);
}
 
int yywrap() {
    return 1;
}

int main(void) {
    current_whitespace_length = 0;
    root_node = init_haml_node("_r00t_", -2, "");
    last_created_node = root_node;
    
    FILE *myfile = fopen("../examples/template.haml", "r");
    if (!myfile) {
        printf("Can't open file\n");
        return -1;
    }
    yyin = myfile;
    yylex();
    fclose(myfile);
    
    print_tree(root_node,0);
}

haml_node_t * init_haml_node(char *tag_name, int ws, char *attrs) {
    haml_node_t * node = malloc(sizeof(haml_node_t));
    node->children = malloc(sizeof(haml_node_t) * 5);
    node->tag_name = strdup(tag_name);
    node->type = 0;
    node->max_children = 5;
    node->child_count = 0;
    node->ws = ws;
    node->attrs = strdup(attrs);
    return node;
}

int append_new_haml_node(haml_node_t *parent, char *tag_name, char *attrs) {
    haml_node_t * new_child = init_haml_node(tag_name, current_whitespace_length, attrs);
    if (parent->child_count == parent->max_children - 1) {
        parent->max_children = parent->max_children + ceil((parent->max_children * 0.33)); // grow by 33%
        parent->children = realloc(parent->children, parent->max_children * sizeof(char *));
    }
    parent->children[parent->child_count++] = new_child;
    new_child->parent = parent;
    return new_child;
}

int create_tag_node(char * haml_string) {
    char * first_char = malloc(sizeof(char));

    strncpy(first_char, haml_string, 1);
    if (strcmp(first_char, "%") == 0) {
        haml_node_t * parent = find_parent(last_created_node);
        last_created_node = append_new_haml_node(parent, haml_string, "");

    } else if (strcmp(first_char, "#") == 0) {
        int i, id_length;
        char *id_string, *full_id_attr;
        char attr_prefix[4] = "id=";
        
        id_length = strlen(haml_string);
        
        id_string = malloc(sizeof(char) * (id_length - 1));  // excluding hash
        full_id_attr = malloc(sizeof(char) * (strlen(id_string) + strlen(attr_prefix)));
        
        for (i=0; i<strlen(haml_string); i++) {
            id_string[i] = haml_string[i+1];  // copy everything except hash
        }
        
        full_id_attr = strcat(attr_prefix, id_string);
        haml_node_t * parent = find_parent(last_created_node);
        last_created_node = append_new_haml_node(parent, "%div", full_id_attr);
    }
    return 0;
}

int create_text_node(char * text) {
    haml_node_t *node;
    node = append_new_haml_node(last_created_node, "", "");
    node->text_contents = strdup(text);
    node->type = 1;
    node->ws = last_created_node->ws + 2;
    return 0;
}

int set_whitespace(char * whitespace_string) {
    current_whitespace_length = strlen(yytext);
    return 0;
}

haml_node_t * find_parent(haml_node_t * current_node) {
    if (current_node->parent == 0) {
        return current_node;
    }
    if (current_node->ws > current_whitespace_length - 2) {
        find_parent(current_node->parent);
    } else {
        return current_node;
    }
}

int print_tree(haml_node_t *current_node, int recursion_depth) {
    int i;
    haml_node_t *n, *c;
    n = current_node;
    char * spaces = malloc(sizeof(char) * current_node->ws);
    
    for (i=0; i<2*recursion_depth; i++) {
        spaces[i] = 0x20;
    }
    recursion_depth++;
    printf("%s<Node#%X tag_name: '%s', type: %i, attrs: '%s', ws: %i, child_count: %i, max_children: %i, text_contents: '%s' children: %X, parent: %X>\n", spaces, n, n->tag_name, n->type, n->attrs, n->ws, n->child_count, n->max_children, n->text_contents, n->children, n->parent);
    for (i=0; i<(n->child_count); i++) {
        c = n->children[i];
        print_tree(c, recursion_depth);
    }
}


//haml_node_t * parse_haml_template(char * name) {}
//char * haml_node_to_html(haml_node_t * root) {}
