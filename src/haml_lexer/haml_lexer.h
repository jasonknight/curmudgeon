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

haml_node_t *init_haml_node(char *, int, char *);
haml_node_t *find_parent(haml_node_t *);
haml_node_t *append_new_haml_node(haml_node_t *, char *, char *);
int create_tag_node(char *);
char *attribuify_special_div_notation(char *);

// File-global variables
static haml_node_t *root_node;
static haml_node_t *last_created_node;
static haml_node_t *last_created_node_indented;
static int current_whitespace_length;

#endif
