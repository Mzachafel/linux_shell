#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXLINE 1000

int main(void)
{
	char line[MAXLINE];
	int status=1;

	do {
		printf("$ ");
		if (fgets(line, MAXLINE, stdin) != NULL)
			system(line);
		else
			status = 0;
	} while (status);

	exit(EXIT_SUCCESS);
}
