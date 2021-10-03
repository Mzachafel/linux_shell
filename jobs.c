#include "jobs.h"

typedef struct _job job;

static pid_t jid=1;
static job *jobs = NULL;

struct _job {
	char *comm;
	pid_t jid;
	pid_t pgid;
	job *next;
};

pid_t jb_create(char *comm, pid_t pgid)
{
	if (jobs == NULL)
		jid = 1;

	job *jb = malloc(sizeof(job));
	jb->comm = strdup(comm);
	jb->jid = jid++;
	jb->pgid = pgid;
	jb->next = NULL;

	if (jobs == NULL) {
		jobs = jb;
	} else {
		job *temp = jobs;
		while (temp->next != NULL)
			temp = temp->next;
		temp->next = jb;
	}
	return jb->jid;
}

void jb_destroy(pid_t jid)
{
	job *temp = jobs;
	if (temp != NULL && temp->jid == jid) {
		jobs = temp->next;
		free(temp->comm);
		free(temp);
		return;
	}
	job *prev;
	while (temp != NULL && temp->jid != jid) {
		prev = temp;
		temp = temp->next;
	}
	if (temp == NULL)
		return;
	prev->next = temp->next;
	free(temp->comm);
	free(temp);
}

void jb_print(void)
{
	job *temp = jobs;
	while (temp != NULL) {
		printf("[%d] %s\n", temp->jid, temp->comm);
		temp = temp->next;
	}
}
