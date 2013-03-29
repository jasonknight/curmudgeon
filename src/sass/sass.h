#ifndef SASS_H
#define SASS_H

typedef struct sass_node sass_node_t;

struct sass_node {
    char *          tag_name;
    short           type; // 0 for html, 1 for text node
    // if we are text node, we grab text_contents, otherwise, we
    // iterate over children and grab their values etc.
    int             ws; // how much whitespace preceded this tag
    int             child_count;
    int             max_children;
    char *          attrs;
    char *          text_contents;
    sass_node_t **  children;
    sass_node_t *   parent;
};

sass_node_t * sass_parse_file(char *name);

#endif
