#include <stdio.h>
#include "util.h"

extern int yyparse();

int main(int argc, char** argv)
{
	yyparse();
	printf("\n");
}
