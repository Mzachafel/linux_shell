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

void sigchldhandler(int sig);

%}

%%
"|" 			 { return PIPE; }
"<" 			 { return READ; }
1?> 			 { return OUTOVERWRITE; }
1?>>			 { return OUTAPPEND; }
2> 			 { return ERROVERWRITE; }
2>>			 { return ERRAPPEND; }
"&"			 { return BACKGROUND; }	   
2>&1                     { return ERRTOOUT; }
%?[a-zA-Z0-9?*_\.\/-]+|"~" { yylval.arg = strdup(yytext); return _ARGUMENT; }
\n			 { return _NEWLINE; }
[ \t]			 { /* ignore whitespaces */ }
.			 { printf("syntax error\n"); }
%%

int main()
{
	signal(SIGCHLD, sigchldhandler);
	yyparse();
	return 0;
}
