%{

#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "exec.h"

int yylex();
void yyerror(const char*);

%}

%union {
	struct _comblocks *cblx;
	struct _commands *coms;
	struct _arguments *args;
	struct _ioredir *ior;
	char *arg;
	int place;
}

%token PIPE READ OUTOVERWRITE OUTAPPEND ERROVERWRITE ERRAPPEND ERRTOOUT 
%token FOREGROUND BACKGROUND _NEWLINE
%token <arg> _ARGUMENT

%type <cblx> comblocks
%type <coms> commands
%type <args> arguments
%type <ior> ioredir
%type <place> placement

%%

comlist: /* nothing */
       | comlist comline
;

comline:
	comblocks _NEWLINE { 
	    execcblx($1);
	}
        | _NEWLINE { }
;

comblocks:
	  commands ioredir placement {
	      comblocks *cblx = creatcblx();
	      $$ = addcbl(cblx, $1, $2, $3);
	  }
	  | comblocks commands ioredir placement {
	      $$ = addcbl($1, $2, $3, $4);
	  }
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

ioredir:
       	{
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

placement:
	 {
	     $$ = 0;
	 }
	 | FOREGROUND {
	     $$ = 0;
	 }
	 | BACKGROUND {
	     $$ = 1;
	 }
;

%%

void yyerror(const char* s)
{
	fprintf(stderr, "error: %s\n", s);
}
