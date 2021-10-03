#include "sortlist.h"

#define DEFMAXSL 8

struct sortlist {
	int maxarg;
	char **args;
	int curarg;
} static *sl;

void creatsl(void)
{
	sl = malloc(sizeof(struct sortlist));
	sl->maxarg = DEFMAXSL;
	sl->args = calloc(sl->maxarg, sizeof(char *));
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

struct arguments *writesl(struct arguments *args)
{
	qsort(sl->args, sl->curarg, sizeof(char *), cmpfunc);
	for (int i=0; i<sl->curarg; i++)
		args = addarg(args, sl->args[i]);

	return args;
}

void clearsl(void)
{
	for (int i=0; i<sl->curarg; i++)
		free(sl->args[i]);
	free(sl);
}
