%{

#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include "parser.tab.h"

#define YY_INPUT(buf,result,max_size) \
{ \
	result = YY_NULL; \
	static char prompt[102]; \
	strcat(getcwd(prompt,100), "$ "); \
	cmdline = readline(prompt); \
	if (cmdline[0] != '\0') \
		add_history(cmdline); \
	sprintf(buf, "%s\n", cmdline); \
	result = strlen(buf); \
}

char *cmdline = NULL;

%}

%%
"|" 			 { return PIPE; }
"<" 			 { return READ; }
">" 			 { return OVERWRITE; }
">>"			 { return APPEND; }
"&"			 { return BACKGROUND; }	   
[a-zA-Z0-9?*_\.\/-]+|"~" { yylval.arg = strdup(yytext); return _ARGUMENT; }
\n			 { return _NEWLINE; }
[ \t]			 { /* ignore whitespaces */ }
.			 { printf("syntax error\n"); }
%%

int main()
{
	signal(SIGCHLD, SIG_IGN);
	yyparse();
	return 0;
}
