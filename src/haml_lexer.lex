%{
    #include "haml_lexer.h"
    #include <stdio.h>
%}
%pointer
%%
[ \t]+                          { printf("WS: %d\n",strlen(yytext)); }    
[\r\n\f]+                       { printf("NLS: %d\n",strlen(yytext)); }    
[%\.#][a-zA-Z\-_]+              { printf("TAG: %s\n",yytext); }    
\{.+\}                          { printf("Options: %s\n",yytext); }    
=.+$                            { printf("Directive: %s\n",yytext); }  
[^ \r\n\f\t\{#%\.]+.+$          { printf("Text: %s\n",yytext); }  
.                               { printf("unknown char %s\n",yytext);}
%%
void yyerror(const char *str) {
  fprintf(stderr,"error: %s\n",str);
}
 
int yywrap() {
  return 1;
}
int main(void) {
    FILE *myfile = fopen("examples/template.haml", "r");
    if (!myfile) {
      printf("Can't open file\n");
      return -1;
    }
    yyin = myfile;
    yylex();
    fclose(myfile);
}

haml_node_t * parse_haml_template(char * name) {}
char * haml_node_to_html(haml_node_t * root) {}
