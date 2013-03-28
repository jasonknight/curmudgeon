%{
    #include "sass.h"
    #include <math.h>
    #include <stdlib.h>
    #include <stdio.h>
%}
%pointer
%%  
[ \t]+                       { haml_set_whitespace(yytext); }
[\r\n\f]+                    { /*printf("NLS: %d\n",strlen(yytext)); */}
[%\.#][a-zA-Z\-_.#]+         { haml_create_tag_node(strdup(yytext)); }
\{.+\}                       { haml_set_options(strdup(yytext)); }
=.+$                 { haml_parse_directive(last_created_node,strdup(yytext)); }
[^ \r\n\f\t\{#%\.]+.*$       { haml_create_text_node(strdup(yytext)); }
.                            { printf("unknown char %s\n",yytext);}
%%

void yyerror(const char *str) {
    fprintf(stderr,"error: %s\n",str);
}
 
int yywrap() {
    return 1;
}

int haml_set_options(char * opts) {
    char *cpy = malloc(sizeof(char) * strlen(opts));
    int i = 1;
    int len = strlen(opts) -1;
    while (i < len) {
        cpy[i-1] = opts[i];
        i++;
    }
    cpy[i] = '\0';
    last_created_node->attrs = realloc(last_created_node->attrs, strlen(last_created_node->attrs) + strlen(opts));
    strcat(last_created_node->attrs,cpy);
    return 0;
}

int haml_parse_directive(haml_node_t * root, char * directive) {
    char * cpy = directive;
    ++cpy;
    char * token = strsep(&cpy," ");
    if (strcmp(token,"include") == 0) {
        yypush_buffer_state(yy_create_buffer( yyin, YY_BUF_SIZE )); 
        char * token = strsep(&cpy," ");
        haml_parse_file(token);
        yypop_buffer_state();
    }
    return 0;
}

haml_node_t * haml_parse_file(char *name) {
    FILE *myfile = fopen(name, "r");
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
    return root_node;
}

haml_node_t * haml_init_node(char *tag_name, int ws, char * id, char * classes) {
    //printf("Created Node: %s with attrs[%s %s]\n",tag_name,id,classes);
    char * fmt = malloc(sizeof(char) * 100);
    if ( strlen(id) > 0 && strlen(classes) > 0) {
        fmt = "id=\"%s\" class=\"%s\"";
    } else if (strlen(id) > 1 && id != "") {
        fmt = "id=\"%s\"";
    } else if (strlen(classes) > 1 && classes != "") {
        fmt = "class=\"%s\"";
    } else {
        fmt = "";
    }
    char * attrs = malloc(sizeof(char) * (strlen(id) + strlen(classes) + strlen(fmt)) );
    sprintf(attrs,fmt,id,classes);
    haml_node_t * node = malloc(sizeof(haml_node_t));
    node->children = malloc(sizeof(haml_node_t) * 5);
    node->tag_name = strdup(tag_name);
    node->type = 0;
    node->max_children = 5;
    node->child_count = 0;
    node->ws = ws;
    node->attrs = attrs;
    return node;
}

haml_node_t * haml_append_new_node(haml_node_t *parent, char *tag_name, char * id,char *classes) {

    haml_node_t * new_child  = haml_init_node(++tag_name, current_whitespace_length, id,classes);
    if (parent->child_count == parent->max_children - 1) {
        parent->max_children = parent->max_children + ceil((parent->max_children * 0.33)); // grow by 33%
        parent->children = realloc(parent->children, parent->max_children * sizeof(char *));
    }
    parent->children[parent->child_count++] = new_child;
    new_child->parent = parent;
    return new_child;
}

int haml_create_tag_node(char * haml_string) {
    if (haml_string[0] == '%') {
        haml_node_t * parent    = haml_find_parent(last_created_node);
        last_created_node       = haml_append_new_node(parent, haml_string, "","");
    } else if (haml_string[0] == '#' || haml_string[0] == '.') {
        char * id_is            = haml_extract_id_from_string(haml_string);
        char * classes_are      = haml_extract_classes_from_string(haml_string);
        haml_node_t * parent    = haml_find_parent(last_created_node);
        last_created_node       = haml_append_new_node(parent, "%div", id_is,classes_are);
    }    // char * first_char = malloc(sizeof(char));
    return 0;
}
char * haml_extract_id_from_string(char * haml_string) {
    //printf("### Extracting id from string %s\n",haml_string);
    char * src = strdup(haml_string); // we will free this later
    char * tmp = src; // working copy
    // result cannot be larger than the existing string
    char * result = malloc(sizeof(char) * strlen(haml_string));
    // while char != #
    while (*tmp != '#' && *tmp != '\0') {
      ++tmp;
    }
    if (*tmp == '\0') {
        return ""; // we reached the null term before finding us an id
    } else {
        ++tmp; // advance one char to get past the #
    }
    int i = 0;
    while(strchr(". ",*tmp) == NULL) {
        result[i] = *tmp;
        ++tmp;
        ++i;
    }
    result[i] = '\0';
    free(src);
    return result;
}
char * haml_extract_classes_from_string(char * haml_string) {
    //printf("... %s\n",haml_string);
    char * buffer = malloc(sizeof(char) * strlen(haml_string));
    int i;
    int j;
    j = 0;
    int in_parse = 0;
    for (i = 0; i < strlen(haml_string); i++) {
     // printf("[%c][%d]",haml_string[i],in_parse);
      if (haml_string[i] == '#') {
        in_parse = 0;
        continue;
      }
      if (in_parse == 0 && haml_string[i] == '.') {
        in_parse = 1;
        if (strlen(buffer) > 0) {
            buffer[j] = ' ';
            j++;
        }
        continue;
      }
      if (in_parse == 1 && haml_string[i] == '.') {
        buffer[j] = ' ';
        j++;
        continue;
      }
      if (in_parse == 1) {
        buffer[j] = haml_string[i];
        j++;
      }
    }
    buffer[j] = '\0';
    //printf("buffer is %s\n", buffer);
    return buffer;
}

int haml_create_text_node(char * text) {
    haml_node_t *node;
    node = haml_append_new_node(last_created_node, "", "", "");
    node->text_contents = strdup(text);
    node->type = 1;
    node->ws = last_created_node->ws + 2;
    return 0;
}

int haml_set_whitespace(char * whitespace_string) {
    current_whitespace_length = strlen(yytext);
    return 0;
}

haml_node_t * haml_find_parent(haml_node_t * current_node) {
    if (current_node->parent == 0) {
        return current_node;
    }
    if (current_node->ws > current_whitespace_length - 2) {
        return haml_find_parent(current_node->parent);
    } else {
        return current_node;
    }
}

char * haml_node_as_xml(haml_node_t * node,int depth) {
    int i = 0;
    char * result = malloc(sizeof(char) * 1 );
    char * tmp;
    char * spaces = " ";
    

    result[0] = '\0';
    for (i = 0; i < node->child_count; i++) {
       char * c_res = haml_node_as_xml(node->children[i],++depth); 
       tmp = haml_append_string_with_newline(spaces,result,c_res);
       free(result);
       free(c_res);
       result = tmp;
    }
    if (node->type == 1) {
       tmp = haml_append_string(spaces,result,node->text_contents); 
       free(result);
       result = tmp;
    } else if (strcmp(node->tag_name,"_r00t_") != 0) {
        if (
            strcmp("meta",node->tag_name)   == 0 ||
            strcmp("input",node->tag_name)  == 0 ||
            strcmp("br",node->tag_name)     == 0 ||
            strcmp("hr",node->tag_name)     == 0 ||
            strcmp("img",node->tag_name)    == 0 ||
            strcmp("link",node->tag_name)   == 0
        ) {
            char * fmt = "<%s %s />\n";
            int len = strlen(node->tag_name) + strlen(node->attrs) + strlen(result) + strlen(fmt) + strlen(spaces);
            char * nres = malloc(sizeof(char) * len);
            sprintf(nres,fmt,node->tag_name,node->attrs);
            return nres;

        } else {
            char * fmt = "<%s %s>%s</%s> ";
            int len = strlen(node->tag_name) + strlen(node->attrs) + strlen(result) + strlen(fmt) + strlen(spaces);
            char * nres = malloc(sizeof(char) * len);
            sprintf(nres,fmt,node->tag_name,node->attrs,result,node->tag_name);
            return nres;
        }
    }
    return result;
}
char * haml_append_string(char * spaces,char * s1, char * s2) {
        char * fmt = "%s%s%s";
        char * new_string = malloc(sizeof(char) * ( strlen(s1) + strlen(s2) + strlen(fmt) ));
        sprintf(new_string,fmt,spaces,s1,s2);
        return new_string;
}
char * haml_append_string_with_newline(char * spaces,char * s1, char * s2) {
        char * fmt = "%s%s\n%s%s";
        char * new_string = malloc(sizeof(char) * ( strlen(s1) + strlen(s2) + strlen(fmt) ));
        sprintf(new_string,fmt,spaces,s1,spaces,s2);
        return new_string;
}

int print_tree(haml_node_t *current_node, int recursion_depth) {
    int i;
    haml_node_t *n, *c;
    n = current_node;
    char * spaces = malloc(sizeof(char) * (current_node->ws));
    
    for (i=0; i<2*recursion_depth; i++) {
        spaces[i] = 0x20;
    }
    recursion_depth++;
    char * fmt = "%s  <Node#%X \n\t%stag_name: '%s', \n\t%stype: %i, \n\t%sattrs: '%s', \t\tws: %i, child_count: %i, max_children: %i, text_contents: '%s' children: %X, parent: %X>\n";
    printf(fmt, spaces, 
                n, 
                spaces, 
                n->tag_name, 
                spaces,
                n->type, 
                spaces,
                n->attrs, 
                n->ws, 
                n->child_count, 
                n->max_children, 
                n->text_contents, 
                n->children, 
                n->parent);
    for (i=0; i<(n->child_count); i++) {
        c = n->children[i];
        print_tree(c, recursion_depth);
    }
}


//haml_node_t * parse_haml_template(char * name) {}
//char * haml_node_to_html(haml_node_t * root) {}
