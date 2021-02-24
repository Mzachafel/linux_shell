%{
#define YYSTYPE char *
#include <stdio.h>
#include <string.h>
int yylex();
void yyerror(const char*);
typedef struct ll {
	char* str;
	struct ll* next;
} list;
typedef struct {
	list args;
	int size;
} command;
%}

%token WORD PIPE NEWLINE

%%
command_list: /* nothing */
	    | command_list command_line
;

command_line: pipe_list NEWLINE
            | NEWLINE /* empty line */
;

pipe_list: pipe_list PIPE comm_and_args
	 | comm_and_args
;

comm_and_args: WORD arg_list { printf("%s\n", $1); }
;

arg_list: arg_list WORD { printf("%s\n", $2); }
	| /* no args */
;
%%

int main(int argc, char** argv)
{
	yyparse();
}

void yyerror(const char* s)
{
	fprintf(stderr, "error: %s\n", s);
}
