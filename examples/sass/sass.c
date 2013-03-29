#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sass.h"

int main(void) {
    char *templatepath[200];
    sprintf(templatepath, "%s/examples/sass/templates/template.sass", get_current_dir_name());
    printf("%s\n\n", templatepath);
    
    //current_whitespace_length = 0;
    //root_node = haml_init_node("_r00t_", -2, "", "");
    //last_created_node = root_node;

    sass_node_t * root_node = sass_parse_file(templatepath); 
    //printf("%s\n", haml_node_as_xml(root_node,1));
}