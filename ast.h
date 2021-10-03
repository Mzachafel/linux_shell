#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include <string.h>

struct arguments {
	int maxarg;
	char **arg_list;
	int curarg;
};
	
struct arguments *creatargs(void);
struct arguments *addarg(struct arguments *, char *);
void clearargs(struct arguments *);

struct commands {
	int maxcom;
	struct arguments **com_list;
	int curcom;
};

struct commands *creatcoms(void);
struct commands *addcom(struct commands *, struct arguments *);
void clearcoms(struct commands *);

#endif
