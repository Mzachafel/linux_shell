%{

#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "exec.h"

int yylex();
void yyerror(const char*);

int background = 0;

%}

%union {
	struct _commands *coms;
	struct _arguments *args;
	struct _ioredir *ior;
	char *arg;
}

%token PIPE READ OUTOVERWRITE OUTAPPEND ERROVERWRITE ERRAPPEND ERRTOOUT BACKGROUND _NEWLINE
%token <arg> _ARGUMENT

%type <coms> commands
%type <args> arguments
%type <ior> ioredir

%%

command_list: /* nothing */
	    | command_list command_line
;

command_line:
	    commands ioredir background _NEWLINE { 
	        execcoms($1, $2);
		clearcoms($1);
		clearior($2);
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

ioredir: {
	    $$ = creatior();
	}
	| ioredir READ _ARGUMENT {
	    $$ = addior($1, 0, 0, $3);
	}
	| ioredir OUTOVERWRITE _ARGUMENT {
	    $$ = addior($1, 1, 0, $3);
	}
	| ioredir OUTAPPEND _ARGUMENT {
	    $$ = addior($1, 1, 1, $3);
	}
	| ioredir ERROVERWRITE _ARGUMENT {
	    $$ = addior($1, 2, 0, $3);
	}
	| ioredir ERRAPPEND _ARGUMENT {
	    $$ = addior($1, 2, 1, $3);
	}
	| ioredir ERRTOOUT {
	    $$ = addior($1, 2, 1, NULL);
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
