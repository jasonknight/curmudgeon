%{
    #include "sass.h"
    #include <math.h>
    #include <stdlib.h>
    #include <stdio.h>
%}
%pointer
%%  
[ \t]+                       { printf("WS           %i\n",strlen(yytext)); }
[\r\n\f]+                    { /*printf("NL: %d\n",strlen(yytext));*/ }
:.*                          { printf("ATTR         %s\n",yytext); }
$.*                          { printf("VAR          %s\n",yytext); }
@.*                          { printf("DIRECTIVE    %s\n",yytext); }
[#\.].*                      { printf("ATTR_NODE    %s\n",yytext); }
[^ \t\r\n\f\:\.#].*          { printf("HTML_NODE    %s\n",yytext); }
.                            { printf("unknown char %s\n",yytext);}
%%

void yyerror(const char *str) {
    fprintf(stderr,"error: %s\n",str);
}
 
int yywrap() {
    return 1;
}


sass_node_t * sass_parse_file(char *name) {
    FILE *myfile = fopen(name, "r");
    if (!myfile) {
        printf("Can't open file '%s'\n",name);
        return NULL;
    } else {
        printf("Opened file %s\n", name);
    }
    yyin = myfile;
    printf("calling yylex\n");
    yylex();
    printf("yylex called\n");
    fclose(myfile);
    return 0;
}