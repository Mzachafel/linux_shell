#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <regex.h>
#include <dirent.h>
#include "ast.h"

extern char *infile;
extern char *outfile;
extern int append;
extern int background;

#define DEFMAXSL 8
struct sortinglist {
	int maxarg;
	char **args;
	int curarg;
} *sl;

void creatsl(void)
{
	sl = (struct sortinglist *) malloc(sizeof(struct sortinglist));
	sl->maxarg = DEFMAXSL;
	sl->args = (char **) calloc(sl->maxarg, sizeof(char *));
	sl->curarg = 0;
}

void addtosl(char *arg)
{
	if (sl->curarg == sl->maxarg) {
		sl->maxarg *= 2;
		sl->args = (char **) realloc(sl->args, sl->maxarg * sizeof(char *));
	}
	sl->args[sl->curarg++] = strdup(arg);
}

int cmpfunc(const void *a, const void *b)
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

void wildcard(char* prefix, char* suffix)
{
	// skip wildcard expansion if no joker symbols in expression
	if (!strcmp(prefix, "") && !strpbrk(suffix, "*?")) {
		addtosl(suffix);
		return;
	}
	// leaf of wildcard expansion
	if (*suffix == '\0') {
		addtosl(prefix);
		return;
	}

	// get next regex in path
	char *reg = (char*) malloc(2*strlen(suffix)+10);
	char *s = suffix;
	char *r = reg;

	if (*s == '/')
		s++;

	*r++ = '^';
	while (*s != '\0' && *s != '/') {
		if (*s == '*') { *r++='.'; *r++='*'; }
		else if (*s == '?') *r++='.';
		else if (*s == '.') { *r++='\\'; *r++='.'; }
		else *r++=*s;
		s++;
	}
	if (*s == '/')
		*s++;
	*r++='$'; *r='\0';

	regex_t preg;
	int rc;

	if ((rc = regcomp(&preg, reg, 0)) != 0)
		return;

	// if path begins with '/' -> absolute, else -> current directory as beginning
	DIR *dir;
	if (!strcmp(prefix, "")) {
		if (suffix[0] == '/')
			dir = opendir("/");
		else
			dir = opendir(".");
	} else
		dir = opendir(prefix);
	if (dir == NULL) {
		regfree(&preg);
		return;
	}

	// expanding wildcard tree
	struct dirent *ent;
	size_t nmatch = 1;
	regmatch_t pmatch[1];
	while ((ent = readdir(dir)) != NULL)
		if (!regexec(&preg, ent->d_name, nmatch, pmatch, 0)) {
			// skip filename with dot in beginnging if wildcard doesn't start with dot
			if (ent->d_name[0] == '.' && suffix[0] != '.')
				continue;
			else {
				char* tmp = (char*) malloc(strlen(prefix)+strlen(ent->d_name)+2);
				strcpy(tmp, prefix); 
				if (strcmp(prefix, "") || suffix[0] == '/') strcat(tmp, "/"); 
				strcat(tmp, ent->d_name);
				wildcard(tmp, s);
				free(tmp);
			}
		}
	regfree(&preg);
	closedir(dir);
}

struct arguments *expandwc(struct arguments *args, char *wc)
{
	creatsl();
	wildcard("", wc);
	args = writesl(args);
	clearsl();

	return args;
}

int builtin(struct commands *coms)
{
	if (!strcmp(coms->com_list[0]->arg_list[0], "cd"))
		if (coms->com_list[0]->curarg > 2 && coms->com_list[0]->arg_list[2] != NULL) {
			printf("cd: Too many arguments\n");
			return 1;
		} else {
			chdir(coms->com_list[0]->arg_list[1]);
			return 1;
		}
	else if (!strcmp(coms->com_list[0]->arg_list[0], "exit"))
		exit(EXIT_SUCCESS);
	return 0;
}

void execcoms(struct commands *coms)
{
	int ret, status, i;

	if (coms->curcom == 0 || builtin(coms))
		return;

	int tmpin = dup(0);
	int tmpout = dup(1);

	int fdin;
	int fdout;
	if (infile)
		fdin = open(infile, O_RDONLY);
	else
		fdin = dup(tmpin);

	for (int i=0; i<coms->curcom; i++) {
		dup2(fdin, 0);
		close(fdin);

		if (i == coms->curcom-1)
			if (outfile)
				if (append)
					fdout = open(outfile, O_WRONLY | O_APPEND | O_CREAT, 0644);
				else
					fdout = creat(outfile, 0644);
			else
				fdout = dup(tmpout);
		else {
			int fdpipe[2];
			pipe(fdpipe);
			fdin = fdpipe[0];
			fdout = fdpipe[1];
		}

		dup2(fdout,1);
		close(fdout);

		if ((ret = fork()) == 0) {
			if (execvp(coms->com_list[i]->arg_list[0], coms->com_list[i]->arg_list) == -1) {
				dup2(tmpout, 1);
				printf("Command '%s' not found\n", coms->com_list[i]->arg_list[0]);
			}
			exit(EXIT_SUCCESS);
		} else if (ret < 0)
			break;
	}

	dup2(tmpin,0);
	dup2(tmpout,1);
	close(tmpin);
	close(tmpout);

	if (!background)
		waitpid(ret, &status, 0);

	return;
}


void clearvars(void)
{
	if (infile) {
		free(infile);
		infile = NULL;
	}
	if (outfile) {
		free(outfile);
		outfile = NULL;
	}
	append = 0; background = 0;
}
