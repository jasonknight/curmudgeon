#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "sass.h"

int main(void) {
    char * templatepath = "examples/sass/templates/template.sass";
    printf("%s\n\n", templatepath);
    sass_node_t * sass_root_node = sass_parse_file(templatepath); 
    sass_print_tree(sass_root_node, 0);
    printf("%s", sass_as_css(sass_root_node, 0));
}
