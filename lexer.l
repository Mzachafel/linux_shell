%{
#include "parser.tab.h"
%}

%%
"|"	   { return PIPE; }
[a-zA-Z-]+ { return WORD; }
\n	   { return NEWLINE; }
[ \t]      { /* ignore whitespaces */ }
. 	   { printf("syntax error\n"); }
%%
