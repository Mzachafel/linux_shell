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
void addarg(char *arg)
{
	if (coms[cur].cur == coms[cur].max) {
		if (!arg)
			coms[cur].max++;
		else
			coms[cur].max *= 2;
		coms[cur].args = realloc(coms[cur].args, coms[cur].max*sizeof(char *));
	}
	coms[cur].args[coms[cur].cur++] = (!arg) ? NULL : strdup(arg);
}


char* infile;
char* outfile;

char currdir[100];

int parseline(char*);
int getword(char*, int, char*);
int expandwildcard(char*, char*);
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
				if (!expandwildcard("", word))
					fputs("Error expanding wildcard\n", stderr);
				break;
			case Pipe:
				addarg(NULL);
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
	addarg(NULL);

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

int expandwildcard(char* prefix, char* suffix)
{
	if (!strcmp(prefix, "") && !strpbrk(suffix, "*?")) {
		addarg(suffix);
		return 1;
	}
	if (*suffix == '\0') {
		addarg(prefix);
		return 1;
	}

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
		return 0;

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
		return 0;
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
					expandwildcard(tmp, s);
					free(tmp);
				}
			}
			else {
				char* tmp = (char*) malloc(strlen(prefix)+strlen(ent->d_name)+2);
				strcpy(tmp, prefix); 
				if (strcmp(prefix, "") || suffix[0] == '/') strcat(tmp, "/"); 
				strcat(tmp, ent->d_name);
				expandwildcard(tmp, s);
				free(tmp);
			}
		}
	regfree(&preg);
	closedir(dir);
	return 1;
}

int execute(void)
{
	int ret, status, i;

	if (coms[0].cur == 0)
		return 1;
	if (!strcmp(coms[0].args[0], "cd"))
		if (coms[0].cur > 2 && coms[0].args[2] != NULL) {
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
