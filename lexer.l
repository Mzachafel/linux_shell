%{

#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include "parser.tab.h"

#define YY_INPUT(buf,result,max_size) \
{ \
	result = YY_NULL; \
	static char prompt[102]; \
	strcat(getcwd(prompt,100), "$ "); \
	char *line = readline(prompt); \
	add_history(line); \
	sprintf(buf, "%s\n", line); \
	free(line); \
	result = strlen(buf); \
}

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

int main()
{
	yyparse();
	printf("\n");
	return 0;
}
