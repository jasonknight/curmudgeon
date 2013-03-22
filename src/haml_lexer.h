#ifndef HAML_LEXER_H
#define HAML_LEXER_H
#include <math.h>
typedef struct haml_node haml_node_t;
struct haml_node {
    char * tag_name;
    short type; // 0 for html, 1 for text node
    // if we are text node, we grab text_contents, otherwise, we
    // iterate over children and grab their values etc.
    int ws; // how much whitespace preceded this tag
    haml_node_t ** children;
    haml_node_t * parent;
    int child_count;
    int max_children;
    char * text_contents;
};
int append_new_haml_node(haml_node_t ** dest,char * tag_name) {
    haml_node_t * parent = *dest; 
    haml_node_t * node = init_haml_node(tag_name);
    if (parent->child_count == parent->max_children - 1) {
        parent->max_children = parent->max_children + ceil((parent->max_children * 0.33)); // grow by 33%
        parent->children = realloc(parent->children,parent->max_children * sizeof(char *));
    }
    parent->children[++parent->child_count] = node;
    node->parent = parent;
    *dest = node; // swap the nodes going forward
    return 0;
}
haml_node_t * init_haml_node(char * tag_name) {
    haml_node_t * node = malloc(sizeof(haml_node_t));
    node->children = malloc(sizeof(haml_node_t) * 5);
    node->tag_name = strdup(tag_name);
    node->type = 0;
    node->max_children = 5;
    node->child_count = -1;
    return node;
}
#endif
