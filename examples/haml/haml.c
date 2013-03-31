#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "haml.h"



int main(void) {
    char * templatepath = "examples/haml/templates/template.haml";
    printf("%s\n\n", templatepath);
    haml_node_t * haml_root_node = haml_parse_file(templatepath); 
    print_tree(haml_root_node);
    printf("%s\n", haml_node_as_xml(haml_root_node,1));
}
