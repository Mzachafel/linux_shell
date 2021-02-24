#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define LINESIZE 1024
#define WORDSIZE 128

char* comm[10];
int commlen=0;

int parseline(char*);
void getword(char*, int, char*);
int execute(void);

int main(int argc, char* argv[])
{
	char line[LINESIZE];

	while (fgets(line, LINESIZE, stdin) != NULL) {
		if (!parseline(line))
			fputs("Error parsing line\n", stderr);
		if (!execute())
			fputs("Error executing command\n", stderr);
	}

	return 0;
}

int parseline(char* line)
{
	char word[WORDSIZE];

	while (*line != '\n') {
		while (*line == ' ')
			line++;
		getword(word, WORDSIZE, line);
		comm[commlen++] = strdup(word);
		while (*line != ' ' && *line != '\n')
			line++;
	}
	comm[commlen+1] = NULL;

	return 1;
}

void getword(char* word, int lim, char* line)
{
	while (--lim && *line != ' ' && *line != '\n')
		*word++ = *line++;
	*word = '\0';
}

int execute(void)
{
	char path[80] = "/bin/";
	int i, status;
	pid_t pid;

	if ((pid = fork()) == -1) {
		return 0;
	} else if (pid == 0) {
		strcat(path, comm[0]);
		execv(path, &comm[1]);
		exit(0);
	} else {
		waitpid(pid, &status, WUNTRACED);
		for (i=0; i<commlen; i++)
			free(comm[i]);
		commlen=0;
		return 1;
	}
}
