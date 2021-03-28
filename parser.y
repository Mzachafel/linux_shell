%{

#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "util.h"

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

%token L G GG NL
%token <arg> ARG

%type <coms> commands
%type <args> arguments

%%

command_list: /* nothing */
	    | command_list command_line
;

command_line:
	    commands io_redir background NL { 
	        execcoms($1);
		clearcoms($1);
		clearvars();
		printprompt();
	    }
            | NL {
		printprompt();
	    }
;

commands:
	 arguments {
	     struct commands *coms = creatcoms();
	     $$ = addcom(coms, $1);
	 }
	 | commands '|' arguments {
	     $$ = addcom($1, $3);
	 }
;

arguments:
	 ARG {
	     struct arguments *args = creatargs();
	     $$ = expandwc(args, $1);
	 }
	 | arguments ARG {
	     $$ = expandwc($1, $2);
	 }
;

io_redir:
	| L ARG {
	    infile = strdup($2);
	}
	| G ARG {
	    outfile = strdup($2);
	}
	| GG ARG {
	    outfile = strdup($2);
	    append = 1;
	}
;

background:
	  | '&' {
	      background = 1;
	  }
;

%%

void yyerror(const char* s)
{
	fprintf(stderr, "error: %s\n", s);
}
