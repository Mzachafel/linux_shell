%{

#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "exec.h"

int yylex();
void yyerror(const char*);

char *infile = NULL;
char *outfile = NULL;
char *errfile = NULL;
int outappend = 0;
int errappend = 0;
int background = 0;

%}

%union {
	struct _commands *coms;
	struct _arguments *args;
	char *arg;
}

%token PIPE READ OUTOVERWRITE OUTAPPEND ERROVERWRITE ERRAPPEND ERRTOOUT BACKGROUND _NEWLINE
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
	     commands *coms = creatcoms();
	     $$ = addcom(coms, $1);
	 }
	 | commands PIPE arguments {
	     $$ = addcom($1, $3);
	 }
;

arguments:
	 _ARGUMENT {
	     arguments *args = creatargs();
	     $$ = addarg(args, $1);
	 }
	 | arguments _ARGUMENT {
	     $$ = expandwc($1, $2);
	 }
;

io_redir:
	| io_redir READ _ARGUMENT {
	    infile = strdup($3);
	}
	| io_redir OUTOVERWRITE _ARGUMENT {
	    outfile = strdup($3);
	}
	| io_redir OUTAPPEND _ARGUMENT {
	    outfile = strdup($3);
	    outappend = 1;
	}
	| io_redir ERROVERWRITE _ARGUMENT {
	    errfile = strdup($3);
	}
	| io_redir ERRAPPEND _ARGUMENT {
	    errfile = strdup($3);
	    errappend = 1;
	}
	| io_redir ERRTOOUT {
	    errfile = outfile;
	    errappend = 1;
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
