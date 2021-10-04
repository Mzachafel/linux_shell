#include "jobs.h"

static pid_t jid=1;
static job *jobs = NULL;

pid_t jb_create(char *comm, pid_t pgid, int npids)
{
	if (jobs == NULL)
		jid = 1;

	job *jb = malloc(sizeof(job));
	jb->comm = strdup(comm);
	jb->jid = jid++;
	jb->pgid = pgid;
	jb->pids = malloc(npids * sizeof(pid_t));
	jb->npids = npids;
	jb->pids[jb->npids++] = pgid;
	jb->state = 0; // 0 - running, 1 - stopped
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

void jb_addpid(pid_t pid)
{
	job *temp = jobs;
	while (temp->next != NULL)
		temp = temp->next;
	temp->pids[temp->npids++] = pid;
}

static int belongs(pid_t pid, pid_t *pids, int npids)
{
	for (int i=0; i<npids; i++)
		if (pid == pids[i])
			return 1;
	return 0;
}

void jb_destroy(pid_t pid)
{
	job *temp = jobs;
	if (temp != NULL && belongs(pid, temp->pids, temp->npids)) {
		jobs = temp->next;
		free(temp->comm);
		free(temp->pids);
		free(temp);
		return;
	}
	job *prev;
	while (temp != NULL && !belongs(pid, temp->pids, temp->npids)) {
		prev = temp;
		temp = temp->next;
	}
	if (temp == NULL)
		return;
	prev->next = temp->next;
	free(temp->comm);
	free(temp->pids);
	free(temp);
}

void jb_print(void)
{
	job *temp = jobs;

	while (temp != NULL) {
		printf("[%d] %s %s\n", temp->jid, getstate(temp->state), temp->comm);
		temp = temp->next;
	}
}

job *jb_getattr(pid_t id, int type)
{ // type: 0 - pid, 1 - jid
	job *temp = jobs;
	if (temp != NULL && (type ? id == temp->jid : belongs(id, temp->pids, temp->npids))) {
		return temp;
	}
	while (temp != NULL && !(type ? id == temp->jid : belongs(id, temp->pids, temp->npids))) {
		temp = temp->next;
	}
	if (temp == NULL)
		return NULL;
	return temp;
}
