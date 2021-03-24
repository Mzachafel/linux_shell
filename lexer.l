%{
#include "parser.tab.h"
%}

%%
"|" { return '|'; }
"<" { return L; }
">" { return G; }
">>" { return GG; }
"&" { return '&'; }	   
[a-zA-Z0-9?*_\.\/-]+|"~" { yylval.arg = strdup(yytext); return ARG; }
\n	   { return NL; }
[ \t]      { /* ignore whitespaces */ }
. 	   { printf("syntax error\n"); }
%%
