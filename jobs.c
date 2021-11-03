#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "jobs.h"

extern int batch;

static job *jobs = NULL;
static pid_t jid=1;
static char *states[] = { "Running", "Stopped", "Done" };

static int belongs(pid_t pid, pid_t *pids, int npids)
{
	for (int i=0; i<npids; i++)
		if (pid == pids[i])
			return 1;
	return 0;
}

job* jb_create(char *comm, pid_t pgid, int npids)
{
	if (jobs == NULL)
		jid = 1;

	job *jb = malloc(sizeof(job));
	jb->comm = strdup(comm);
	jb->jid = jid++;
	jb->pgid = pgid;
	jb->pids = malloc(npids * sizeof(pid_t));
	jb->npids = npids;
	jb->state = 0; // 0 - running, 1 - stopped, 2 - terminated, 3 - done
	jb->next = NULL;

	if (jobs == NULL) {
		jobs = jb;
	} else {
		job *temp = jobs;
		while (temp->next != NULL)
			temp = temp->next;
		temp->next = jb;
	}
	return jb;
}

job* jb_get(pid_t id, int type)
{ // type: 0 - pid, 1 - jid
	job *jb = jobs;
	while (jb != NULL && !(type ? id == jb->jid : belongs(id, jb->pids, jb->npids))) {
		jb = jb->next;
	}
	return jb;
}

void jb_printone(job *jb)
{
	if (!batch)
		printf("[%d]  %-10s  %s\n", jb->jid, states[jb->state], jb->comm);
}

void jb_printall(void)
{
	job *jb = jobs, *temp;
	while (jb != NULL) {
		jb_printone(jb);
		temp = jb;
		jb = jb->next;
		if (temp->state == 2)
			jb_destroy(temp);
	}
}

void jb_destroy(job *jb)
{
	job *temp = jobs;
	if (temp != NULL && jb->jid == temp->jid) {
		jobs = temp->next;
	} else {
		job *prev;
		while (temp != NULL && jb->jid != temp->jid) {
			prev = temp;
			temp = temp->next;
		}
		if (temp == NULL)
			return;
		prev->next = temp->next;
	}
	free(temp->comm);
	free(temp->pids);
	free(temp);
}
