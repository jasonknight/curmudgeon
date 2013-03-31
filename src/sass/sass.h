#ifndef SASS_H
#define SASS_H

typedef struct sass_node sass_node_t;
typedef struct sass_attr sass_attr_t;

struct sass_node {
    char           *selector_name;
    int             ws; /**< how much whitespace preceded this tag */
    int             child_count;
    int             max_children;
    int             attr_count;
    int             max_attrs;
    sass_attr_t     **attrs;
    sass_node_t     **children;
    sass_node_t     *parent;
};

struct sass_attr {
    char           *key;
    char           *val;
};

sass_node_t    *sass_parse_file(char *name);
sass_node_t    *sass_init_node(char *selector_name, int ws);
sass_node_t    *sass_find_parent(sass_node_t * current_node);
sass_node_t    *sass_append_new_node(sass_node_t *parent, char *selector_name);
int             sass_append_new_attr(char *attr_string);
int             sass_create_selector_node(char *sass_string);
int             sass_print_tree(sass_node_t *current_node, int recursion_depth);
char           *sass_append_string_with_newline(char *spaces, char *s1, char *s2);
char           *sass_as_css(sass_node_t *node, int depth);

// File-global variables
static sass_node_t *sass_root_node;
static sass_node_t *sass_last_created_node;
static sass_node_t *sass_last_created_node_indented;
static int          sass_current_whitespace_length;

#endif
