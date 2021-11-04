%{

#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "exec.h"

#define YYERROR_VERBOSE

int yylex();
void yyerror(const char*);
extern char* cmdline;

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
%type <place> midplace finplace

%destructor { clearcoms($$); } commands
%destructor { clearior($$); } ioredir
%destructor { free($$); } _ARGUMENT

%%

comlist:
       | comlist comline
;

comline:
	comblocks finplace _NEWLINE {
	    setplace($1, $2);
	    execcblx($1);
	}
        | _NEWLINE {
	    free(cmdline);
	}
	| error _NEWLINE {
	    free(cmdline);
	}
;

comblocks:
	  commands ioredir {
	      comblocks *cblx = creatcblx();
	      $$ = addcbl(cblx, $1, $2);
	  }
	  | comblocks midplace commands ioredir {
	      setplace($1, $2);
	      $$ = addcbl($1, $3, $4);
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

midplace:
	FOREGROUND {
	    $$ = 0;
	}
	| BACKGROUND {
	    $$ = 1;
	}
;

finplace:
	{
	    $$ = 0;
	}
	| midplace {
	    $$ = $1;
	}
;

%%

void yyerror(const char* s)
{
	fprintf(stderr, "mzsh: parsing: %s\n", s);
}
