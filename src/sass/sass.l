%{
    #include <math.h>
    #include <stdlib.h>
    #include <stdio.h>
    #include "sass.h"
%}
%pointer
%%
^[ \t]*$                     { /* ignoring empty line */ }
[ \t]*                       { sass_set_whitespace(yytext);}
[\r\n\f]+                    { /* we ignore those */ }
:.*                          { sass_append_new_attr(strdup(yytext)); }
$.*                          { /*printf("VAR          %s\n",yytext);*/ }
@.*                          { /*printf("DIRECTIVE    %s\n",yytext);*/ }
[#\.].*                      { /*printf("ATTR_NODE    %s\n",yytext);*/ }
[^ \t\r\n\f\:\.#].*          { sass_create_selector_node(strdup(yytext)); }
.                            { /*printf("unknown char %s\n",yytext);*/}
%%

void yyerror(const char *str) {
    fprintf(stderr,"error: %s\n",str);
}
 
int yywrap() {
    return 1;
}

int sass_set_whitespace(char * whitespace_string) {
    printf("ws length is %i\n", strlen(whitespace_string));
    sass_current_whitespace_length = strlen(yytext);
    return 0;
}

sass_node_t *sass_parse_file(char *name) {
    FILE *myfile = fopen(name, "r");
    if (!myfile) {
        printf("Can't open file '%s'\n",name);
        return NULL;
    } else {
        printf("Opened file %s\n", name);
    }
    if ( ! sass_root_node ) {
        sass_root_node = sass_init_node("_r00t_", -2);
        sass_last_created_node = sass_root_node;
        //printf("creating root node %X since not present\n", sass_root_node);
    }
    yyin = myfile;
    //printf("calling yylex\n");
    yylex();
    //printf("yylex called\n");
    fclose(myfile);
    return sass_root_node;
}

int sass_create_selector_node(char *sass_string) {
    printf("\nsass_create_tag_node, sass string is %s\n", sass_string);
    
    sass_node_t *parent;
    
    parent = sass_find_parent(sass_last_created_node);
    //printf("sass_find_parent says: %X\n", parent);
    sass_last_created_node = sass_append_new_node(parent, sass_string);
    sass_current_whitespace_length = 0;
    return 0;
}

sass_node_t *sass_find_parent(sass_node_t *current_node) {
    printf(" sass_find_parent:  %X\n", current_node);
    if (current_node->selector_name == "_r00t_") {
        return current_node;
    }
    if (current_node->ws > sass_current_whitespace_length - 2) {
        return sass_find_parent(current_node->parent);
    } else {
        return current_node;
    }
}

sass_node_t * sass_init_node(char *selector_name, int ws) {
    //printf("    sass_init_node %s %i\n", selector_name, ws);
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

sass_attr_t * sass_init_attr(char *key, char *val) {  
    sass_attr_t *attr = malloc(sizeof(sass_attr_t));
    attr->key       = malloc(sizeof(char) * strlen(key));
    attr->val       = malloc(sizeof(char) * strlen(val));
    attr->key       = strdup(key);
    attr->val       = strdup(val);
    return attr;
}

sass_node_t * sass_append_new_node(sass_node_t *parent, char *selector_name) {
    sass_node_t *new_child;
    
    //printf("   sass_append_new_node %X, %s\n", parent, selector_name);
    new_child = sass_init_node(selector_name, sass_current_whitespace_length);
    //printf("   sass_append_new_node: new_child %X\n", new_child);
    if (parent->child_count == parent->max_children - 1) {
        parent->max_children = parent->max_children + ceil((parent->max_children * 0.33)); // grow by 33%
        parent->children = realloc(parent->children, parent->max_children * sizeof(sass_node_t));
    }
    parent->children[parent->child_count++] = new_child;
    new_child->parent = parent;
    return new_child;
}

int sass_append_new_attr(char *attr_string) {
    printf("sass_append_new_attr %s\n", attr_string);
    sass_node_t *n;
    sass_attr_t *new_attr;
    char * token;
    n = sass_last_created_node; // use shorter variable name from here on
    
    token = strsep(&attr_string, " ");
    
    //printf("sass_append_new_attr: %s\n", attr_string);
    //printf("token is: %s\n", token);
    //printf("rest is: %s\n", attr_string);
    
    new_attr = sass_init_attr(token, ++attr_string);
    
    // expand the size of maximum possible attrs
    if (n->attr_count == n->max_attrs -1) {
        printf("expand the size of maximum possible attrs");
        n->max_attrs = n->max_attrs + ceil((n->max_attrs * 0.33));
        n->attrs = realloc(n->attrs, n->max_attrs * sizeof(sass_attr_t));
    }
    sass_current_whitespace_length = 0;
}

char *sass_as_css(sass_node_t *node, int depth) {
    char *result = malloc(sizeof(char));
    char *tmp;
    char *spaces = " ";
    int i;
    result[0] = 0x00;
    
    for( i=0; i<node->child_count; i++ ) {
      //printf("as_css %i %s %i\n", depth, node->selector_name, node->child_count);
      char *children_result = sass_as_css(node->children[i], ++depth);
      tmp = sass_append_string_with_newline(spaces, result, children_result);
      free(result);
      free(children_result);
      result = tmp;
    }
    
    if( strcmp(node->selector_name, "_r00t_") != 0) {
      char *fmt = "%s { %s }\n";
      int len = strlen(node->selector_name) + strlen(result) + strlen(fmt) + strlen(spaces);
      char *nres = malloc(sizeof(char) * len);
      sprintf(nres, fmt, node->selector_name, result);
      //printf("sss   %s", nres);
      return nres;
    } else {
      return result; // when this function is called with the root node
    }
}

char *sass_append_string_with_newline(char *spaces, char *s1, char *s2) {
    char *fmt = "%s%s\n%s%s";
    char *new_string = malloc(sizeof(char) * ( strlen(s1) + strlen(s2) + strlen(fmt) ));
    sprintf(new_string, fmt, spaces, s1, spaces, s2);
    return new_string;
}


int sass_print_tree(sass_node_t *current_node, int recursion_depth) {
    int i;
    sass_node_t *n, *c;
    n = current_node;
    char *spaces = malloc(sizeof(char) * 100);

    for (i=0; i<2*recursion_depth; i++) {
        spaces[i] = 0x20;
    }
    spaces[i] = 0x00;
    recursion_depth++;


    char * fmt = "%s<Node#%X selector_name: '%s', child_count: %i, attr_count: %i, ws: %i, parent: %X>\n";
    printf(fmt, spaces, 
                n,
                n->selector_name,
                n->child_count,
                n->attr_count,
                n->ws,
                n->parent
                );
    
    for (i=0; i<(n->child_count); i++) {
        c = n->children[i];
        sass_print_tree(c, recursion_depth);
    }
}