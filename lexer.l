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
"|"		 { return PIPE; }
"<" 		 { return READ; }
1?> 		 { return OUTOVERWRITE; }
1?>>		 { return OUTAPPEND; }
2> 		 { return ERROVERWRITE; }
2>>		 { return ERRAPPEND; }
";" 		 { return FOREGROUND; }
"&"		 { return BACKGROUND; }	   
2>&1             { return ERRTOOUT; }
"\n"		 { return _NEWLINE; }
[^|<>;&\n\t ]+   { yylval.arg = strdup(yytext); return _ARGUMENT; }
[ \t]		 { /* ignore whitespace */ }
%%

int main(int argc, char* argv[])
{
	if (argc > 2) {
		fprintf(stderr, "mzsh: too many arguments\n");
		exit(EXIT_FAILURE);
	} else if (argc == 2) {
		int fd;
		size_t n;
		int result;
		if ((fd = open(argv[1], O_RDONLY)) == -1) {
			perror("mzsh: batchmode init");
			exit(EXIT_FAILURE); }
		if (dup2(fd, STDIN_FILENO) == -1) {
			perror("mzsh: batchmode init");
			exit(EXIT_FAILURE); }
		if (close(fd) == -1) {
			perror("mzsh: batchmode init");
			exit(EXIT_FAILURE); }
		batch = 1;
		cmdline = NULL;
		result = getline(&cmdline, &n, stdin);
		if (result == -1 || !strstr(cmdline, "#!") && !strstr(cmdline, "mzsh")) {
			fprintf(stderr, "mzsh: file is not a supported script\n");
			exit(EXIT_FAILURE); }
		free(cmdline);
	}
	if (signal(SIGCHLD, sigchldhandler) == SIG_ERR) {
		perror("mzsh: init");
		exit(EXIT_FAILURE); }
	yyparse();
	return 0;
}
