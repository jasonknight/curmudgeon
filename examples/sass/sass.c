#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "sauce.h"

int main(void) {
    char * templatepath = "examples/sauce/templates/template.sauce";
    printf("%s\n\n", templatepath);
    sauce_node_t * sauce_root_node = sauce_parse_file(templatepath); 
    sauce_print_tree(sauce_root_node, 0);
    printf("%s", sauce_as_css(sauce_root_node, 0));
}
