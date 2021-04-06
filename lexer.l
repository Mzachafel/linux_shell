%{

#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include "parser.tab.h"

#define YY_INPUT(b,r,s) r = my_input(b, s)

int my_input(char *b, int s)
{
	char *line;
	if (feof(yyin)) return YY_NULL;
	char cwd[100];
	char prompt[110];
	sprintf(prompt, "%s$ ", getcwd(cwd, 100));
	line = readline(prompt);
	if (!line) return YY_NULL;
	if (strlen(line) > s-2) return YY_NULL;
	sprintf(b, "%s\n", line);
	add_history(line);
	free(line);
	return strlen(b);
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
