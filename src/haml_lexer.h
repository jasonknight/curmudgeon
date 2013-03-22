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
//int append_new_haml_node(haml_node_t ** dest,char * tag_name) {
//    haml_node_t * root = *dest; 
//    haml_node_t * node = init_haml_node(tag_name);
//    if (root->child_count == root->max_children - 1) {
//        root->max_children = root->max_children + ceil((root->max_children * 0.33)); // grow by 33%
//        root->children = realloc(root->children,root->max_children);
//    }
//    root->children[++root->child_count] = node;
//    node->parent = root;
//    *dest = node; // swap the nodes going forward
//    return 0;
//}
//haml_node_t * init_haml_node(char * tag_name) {
//    haml_node_t * node = malloc(sizeof(haml_node_t));
//    node->children = malloc(sizeof(haml_node_t) * 5);
//    node->tag_name = strdup(tag_name);
//    node->type = 0;
//    node->max_children = 5;
//    node->child_count = -1;
//    return node;
//}
#endif
