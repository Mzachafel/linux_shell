%{

#include "exec.h"

int yylex();
void yyerror(const char*);

char *infile = NULL;
char *outfile = NULL;
int append = 0;
int background = 0;

%}

%union {
	struct commands *coms;
	struct arguments *args;
	char *arg;
}

%token PIPE READ OVERWRITE APPEND BACKGROUND _NEWLINE
%token <arg> _ARGUMENT

%type <coms> commands
%type <args> arguments

%%

command_list: /* nothing */
	    | command_list command_line
;

command_line:
	    commands io_redir background _NEWLINE { 
	        execcoms($1);
		clearcoms($1);
	    }
            | _NEWLINE { }
;

commands:
	 arguments {
	     struct commands *coms = creatcoms();
	     $$ = addcom(coms, $1);
	 }
	 | commands PIPE arguments {
	     $$ = addcom($1, $3);
	 }
;

arguments:
	 _ARGUMENT {
	     struct arguments *args = creatargs();
	     $$ = addarg(args, $1);
	 }
	 | arguments _ARGUMENT {
	     $$ = expandwc($1, $2);
	 }
;

io_redir:
	| READ _ARGUMENT {
	    infile = strdup($2);
	}
	| OVERWRITE _ARGUMENT {
	    outfile = strdup($2);
	}
	| APPEND _ARGUMENT {
	    outfile = strdup($2);
	    append = 1;
	}
;

background:
	  | BACKGROUND {
	      background = 1;
	  }
;

%%

void yyerror(const char* s)
{
	fprintf(stderr, "error: %s\n", s);
}
