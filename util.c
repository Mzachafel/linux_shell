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

char cwd[100];

void printprompt(void)
{
	printf("%s$ ", getcwd(cwd, 100));
}

struct arguments *expandwc(struct arguments *args, char* prefix, char* suffix)
{
	if (!strcmp(prefix, "") && !strpbrk(suffix, "*?"))
		return addarg(args, suffix);
	if (*suffix == '\0')
		return addarg(args, prefix);

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
		return NULL;

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
		return NULL;
	}

	struct dirent *ent;
	size_t nmatch = 1;
	regmatch_t pmatch[1];
	while ((ent = readdir(dir)) != NULL)
		if (!regexec(&preg, ent->d_name, nmatch, pmatch, 0)) {
			if (ent->d_name[0] == '.') {
				if (suffix[0] == '.') {
					char* tmp = (char*) malloc(strlen(prefix)+strlen(ent->d_name)+2);
					strcpy(tmp, prefix); 
					if (strcmp(prefix, "") || suffix[0] == '/') strcat(tmp, "/"); 
					strcat(tmp, ent->d_name);
					args = expandwc(args, tmp, s);
					free(tmp);
				}
			}
			else {
				char* tmp = (char*) malloc(strlen(prefix)+strlen(ent->d_name)+2);
				strcpy(tmp, prefix); 
				if (strcmp(prefix, "") || suffix[0] == '/') strcat(tmp, "/"); 
				strcat(tmp, ent->d_name);
				args = expandwc(args, tmp, s);
				free(tmp);
			}
		}
	regfree(&preg);
	closedir(dir);
	return args;
}

void execcoms(struct commands *coms)
{
	int ret, status, i;

	if (coms->curcom == 0)
		return;
	if (!strcmp(coms->com_list[0]->arg_list[0], "cd"))
		if (coms->com_list[0]->curarg > 2 && coms->com_list[0]->arg_list[2] != NULL) {
			printf("cd: Too many arguments\n");
			return;
		} else {
			chdir(coms->com_list[0]->arg_list[1]);
			return;
		}

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

		switch (ret = fork()) {
			case -1:
				return;
			case 0:
				execvp(coms->com_list[i]->arg_list[0], coms->com_list[i]->arg_list);
				exit(EXIT_SUCCESS);
		}
	}

	dup2(tmpin,0);
	dup2(tmpout,1);
	close(tmpin);
	close(tmpout);

	if (!background)
		waitpid(ret, &status, WUNTRACED);

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
