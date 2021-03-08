#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <regex.h>
#include <dirent.h>

#define LINESIZE 1024
#define WORDSIZE 128

enum token { Num, Pipe, In, Out };

#define COMMAX 8
typedef struct {
	char **args;
	int max;
	int cur;
} Command;

#define COMTABMAX 4
Command *coms;
int max;
int cur;

void creatcom(void)
{
	coms[cur].args = (char **) malloc(COMMAX*sizeof(char *));
	coms[cur].max = COMMAX;
	coms[cur].cur = 0;
}

char* infile;
char* outfile;

char currdir[100];

int parseline(char*);
int getword(char*, int, char*);
int expandwildcard(char*);
int execute(void);
int clearcoms(void);

int main(int argc, char* argv[])
{
	char line[LINESIZE];

	printf("%s$ ", getcwd(currdir, 100));
	while (fgets(line, LINESIZE, stdin) != NULL) {
		if (!parseline(line))
			fputs("Error parsing line\n", stderr);
		if (!execute())
			fputs("Error executing command\n", stderr);
		if (!clearcoms())
			fputs("Error freeing memory\n", stderr);
		printf("%s$ ", getcwd(currdir, 100));
	}
	printf("\n");

	return 0;
}

int parseline(char* line)
{
	char word[WORDSIZE];

	coms = (Command *) malloc(COMTABMAX*sizeof(Command));
	max = COMTABMAX;
	cur = 0;

	creatcom();

	while (*line != '\n') {
		while (*line == ' ')
			line++;
		switch (getword(word, WORDSIZE, line)) {
			case Num:
				if (!expandwildcard(word))
					fputs("Error executing regex\n", stderr);
				break;
			case Pipe:
				if (coms[cur].cur == coms[cur].max) {
					coms[cur].max++;
					coms[cur].args = realloc(coms[cur].args, coms[cur].max*sizeof(char *));
				}
				coms[cur].args[coms[cur].cur] = NULL;
				cur++;
				if (cur == max) {
					max *= 2;
					coms = realloc(coms, max*sizeof(Command));
				}
				creatcom();
				break;
			case In:
				line++;
				while (*line == ' ')
					line++;
				getword(word, WORDSIZE, line);
				infile = strdup(word);
				break;
			case Out:
				line++;
				while (*line == ' ')
					line++;
				getword(word, WORDSIZE, line);
				outfile = strdup(word);
				break;
			default:
				return 0;
		}
		while (*line != ' ' && *line != '\n')
			line++;
	}
	if (coms[cur].cur == coms[cur].max) {
		coms[cur].max++;
		coms[cur].args = realloc(coms[cur].args, coms[cur].max*sizeof(char *));
	}
	coms[cur].args[coms[cur].cur] = NULL;

	return 1;
}

int getword(char* word, int lim, char* line)
{
	if (*line == '|')
		return Pipe;
	if (*line == '<')
		return In;
	if (*line == '>')
		return Out;
	while (--lim && *line != ' ' && *line != '\n') {
		*word++ = *line++;
	}
	*word = '\0';
	return Num;
}

int expandwildcard(char* arg)
{
	if (strpbrk(arg, "*?") == NULL) {
		if (coms[cur].cur == coms[cur].max) {
			coms[cur].max *= 2;
			coms[cur].args = realloc(coms[cur].args, coms[cur].max*sizeof(char *));
		}
		coms[cur].args[coms[cur].cur++] = strdup(arg);
		return 1;
	}

	char *reg = (char*) malloc(2*strlen(arg)+10);
	char *a = arg;
	char *r = reg;

	*r++ = '^';
	while (*a) {
		if (*a == '*') { *r++='.'; *r++='*'; }
		else if (*a == '?') *r++='.';
		else if (*a == '.') { *r++='\\'; *r++='.'; }
		else *r++=*a;
		a++;
	}
	*r++='$'; *r='\0';

	regex_t preg;
	int rc;

	if ((rc = regcomp(&preg, reg, 0)) != 0)
		return 0;

	DIR *dir = opendir(".");
	if (dir == NULL)
		return 0;

	struct dirent *ent;
	size_t nmatch = 1;
	regmatch_t pmatch[1];
	while ((ent = readdir(dir)) != NULL)
		if (!regexec(&preg, ent->d_name, nmatch, pmatch, 0)) {
			if (coms[cur].cur == coms[cur].max) {
				coms[cur].max *= 2;
				coms[cur].args = realloc(coms[cur].args, coms[cur].max*sizeof(char *));
			}
			coms[cur].args[coms[cur].cur++] = strdup(ent->d_name);
		}
	closedir(dir);
	return 1;
}

int execute(void)
{
	int ret, status, i;

	if (coms[0].cur == 0)
		return 1;
	if (!strcmp(coms[0].args[0], "cd"))
		if (coms[0].cur > 2) {
			printf("cd: Too many arguments\n");
			return 1;
		} else {
			chdir(coms[0].args[1]);
			return 1;
		}

	int tmpin = dup(0);
	int tmpout = dup(1);

	int fdin;
	int fdout;
	if (infile)
		fdin = open(infile, O_RDONLY);
	else
		fdin = dup(tmpin);

	for (int i=0; i<=cur; i++) {
		dup2(fdin, 0);
		close(fdin);

		if (i == cur)
			if (outfile)
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
				return 0;
			case 0:
				execvp(coms[i].args[0], coms[i].args);
				exit(EXIT_SUCCESS);
		}
	}

	dup2(tmpin,0);
	dup2(tmpout,1);
	close(tmpin);
	close(tmpout);

	waitpid(ret, &status, WUNTRACED);

	return 1;
}

int clearcoms(void)
{
	int i, j;

	for (i=0; i<=cur; i++)
		for (j=0; j<coms[i].cur; j++) {
			free(coms[i].args[j]);
			coms[i].max = COMMAX;
			coms[i].cur = 0;
		}
	free(coms);
	if (infile) {
		free(infile);
		infile = NULL;
	}
	if (outfile) {
		free(outfile);
		outfile = NULL;
	}

	return 1;
}
