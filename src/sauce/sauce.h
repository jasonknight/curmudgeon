#ifndef SASS_H
#define SASS_H

typedef struct sauce_node sauce_node_t;
typedef struct sauce_attr sauce_attr_t;

struct sauce_node {
    char           *selector_name;
    int             ws; /**< how much whitespace preceded this tag */
    int             child_count;
    int             max_children;
    int             attr_count;
    int             max_attrs;
    sauce_attr_t     **attrs;
    sauce_node_t     **children;
    sauce_node_t     *parent;
};

struct sauce_attr {
    char           *key;
    char           *val;
};

sauce_node_t    *sauce_parse_file(char *name);
sauce_node_t    *sauce_init_node(char *selector_name, int ws);
sauce_node_t    *sauce_find_parent(sauce_node_t * current_node);
sauce_node_t    *sauce_append_new_node(sauce_node_t *parent, char *selector_name);
int             sauce_append_new_attr(char *attr_string);
int             sauce_create_selector_node(char *sauce_string);
int             sauce_print_tree(sauce_node_t *current_node, int recursion_depth);
char           *sauce_append_string_with_newline(char *spaces, char *s1, char *s2);
char           *sauce_as_css(sauce_node_t *node, int depth);

// File-global variables
static sauce_node_t *sauce_root_node;
static sauce_node_t *sauce_last_created_node;
static sauce_node_t *sauce_last_created_node_indented;
static int          sauce_current_whitespace_length;

#endif
