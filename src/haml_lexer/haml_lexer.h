#ifndef HAML_LEXER_H
#define HAML_LEXER_H

typedef struct haml_node haml_node_t;
struct haml_node {
    char *tag_name;
    short type; // 0 for html, 1 for text node
    // if we are text node, we grab text_contents, otherwise, we
    // iterate over children and grab their values etc.
    int ws; // how much whitespace preceded this tag
    int child_count;
    int max_children;
    char *attrs;
    char *text_contents;
    haml_node_t **children;
    haml_node_t *parent;
};
haml_node_t * parse_file(char * name);
haml_node_t *init_haml_node(char * tag_name, int ws, char * id,char * classes);
haml_node_t *find_parent(haml_node_t *);
haml_node_t *append_new_haml_node(haml_node_t *, char *, char *,char *);
int haml_parse_directive(haml_node_t * root,char * directive);
int create_tag_node(char *);
char *attribuify_special_div_notation(char *);
char * extract_id_from_string(char * haml_string);
char * extract_classes_from_string(char * haml_string);

// File-global variables
static haml_node_t *root_node;
static haml_node_t *last_created_node;
static haml_node_t *last_created_node_indented;
static int current_whitespace_length;

#endif
