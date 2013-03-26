#ifndef HAML_LEXER_H
#define HAML_LEXER_H

typedef struct haml_node haml_node_t;
struct haml_node {
    char *          tag_name;
    short           type; // 0 for html, 1 for text node
    // if we are text node, we grab text_contents, otherwise, we
    // iterate over children and grab their values etc.
    int             ws; // how much whitespace preceded this tag
    int             child_count;
    int             max_children;
    char *          attrs;
    char *          text_contents;
    haml_node_t **  children;
    haml_node_t *   parent;
};

haml_node_t *   haml_parse_file(char *name);
haml_node_t *   haml_init_node(char *tag_name, int ws, char *id, char *classes);
haml_node_t *   haml_find_parent(haml_node_t *);
haml_node_t *   haml_append_new_node(haml_node_t *, char *, char *,char *);
int             haml_parse_directive(haml_node_t * root,char * directive);
int             haml_set_options(char * opts);
int             haml_create_tag_node(char *);
char *          haml_attribuify_special_div_notation(char *);
char *          haml_extract_id_from_string(char * haml_string);
char *          haml_extract_classes_from_string(char * haml_string);
char *          haml_node_as_xml(haml_node_t * node,int depth);
char *          haml_append_string(char * spaces,char * s1, char * s2);
char *          haml_append_string_with_newline(char * spaces,char * s1, char * s2);
// File-global variables
haml_node_t *    haml_root_node;
haml_node_t *    haml_last_created_node;
haml_node_t *    haml_last_created_node_indented;
int              haml_current_whitespace_length;

#endif
