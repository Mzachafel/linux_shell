%{
#define YYSTYPE char *
#include "parser.tab.h"
%}

%%
"|"	   { return PIPE; }
[a-zA-Z0-9-]+ { yylval = strdup(yytext); return WORD; }
\n	   { return NEWLINE; }
[ \t]      { /* ignore whitespaces */ }
. 	   { printf("syntax error\n"); }
%%
