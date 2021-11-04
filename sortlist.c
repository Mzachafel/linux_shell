#include <stdlib.h>
#include <string.h>
#include "sortlist.h"

typedef struct _sortlist {
	char **args;
	int maxarg;
	int curarg;
} sortlist;

static sortlist *sl;

#define DEFMAXSL 8

void creatsl(void)
{
	sl = malloc(sizeof(sortlist));
	sl->args = malloc(DEFMAXSL * sizeof(char *));
	sl->maxarg = DEFMAXSL;
	sl->curarg = 0;
}

void addtosl(char *arg)
{
	if (sl->curarg == sl->maxarg) {
		sl->maxarg *= 2;
		sl->args = realloc(sl->args, sl->maxarg * sizeof(char *));
	}
	sl->args[sl->curarg++] = strdup(arg);
}

static int cmpfunc(const void *a, const void *b)
{
	const char **ia = (const char **) a;
	const char **ib = (const char **) b;
	return strcmp(*ia, *ib);
}

arguments* writesl(arguments *args)
{
	qsort(sl->args, sl->curarg, sizeof(char *), cmpfunc);
	for (int i=0; i<sl->curarg; i++)
		args = addarg(args, sl->args[i]);
	return args;
}

void clearsl(void)
{
	free(sl->args);
	free(sl);
}
