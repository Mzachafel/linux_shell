%{

#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "parser.tab.h"

#define YY_INPUT(buf,result,max_size) \
{ \
	if (batch) { \
		cmdline = NULL; \
		static size_t n = 0; \
		result = getline(&cmdline, &n, stdin); \
		if (result == -1) \
			exit(EXIT_SUCCESS); \
		sprintf(buf, "%s", cmdline); \
	} \
	else { \
		result = YY_NULL; \
		static char prompt[102]; \
		strcat(getcwd(prompt,100), "$ "); \
		cmdline = readline(prompt); \
		if (cmdline[0] != '\0') \
			add_history(cmdline); \
		sprintf(buf, "%s\n", cmdline); \
		result = strlen(buf); \
	} \
}

char *cmdline = NULL;
int batch = 0;

void sigchldhandler(int sig);

%}

%%
"|" 			 { return PIPE; }
"<" 			 { return READ; }
1?> 			 { return OUTOVERWRITE; }
1?>>			 { return OUTAPPEND; }
2> 			 { return ERROVERWRITE; }
2>>			 { return ERRAPPEND; }
; 			 { return FOREGROUND; }
&			 { return BACKGROUND; }	   
2>&1                     { return ERRTOOUT; }
%?[a-zA-Z0-9?*_\.\/-]+|"~" { yylval.arg = strdup(yytext); return _ARGUMENT; }
\n			 { return _NEWLINE; }
[ \t]			 { /* ignore whitespaces */ }
.			 { printf("syntax error\n"); }
%%

int main(int argc, char* argv[])
{
	int fd;
	if (argc == 2) {
		if ((fd = open(argv[1], O_RDONLY)) == -1) {
			perror("open");
			exit(EXIT_FAILURE);
		}
		dup2(fd, STDIN_FILENO);
		close(fd);
		batch = 1;
	}
	signal(SIGCHLD, sigchldhandler);
	yyparse();
	return 0;
}
