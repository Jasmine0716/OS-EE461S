#ifndef JOB_H
#define JOB_H

#include<stdlib.h>

typedef struct Jobs{
	pid_t pg_id;
	int job_id;
	bool in_fg;
	bool in_bg;
	bool in_stop;
	char *input_cmd;
	struct Jobs *next_job;
	struct Jobs *prev_job;
}Job;

void free_job(Job *job);
Job* create_job(pid_t pgid, int status, char* input);
void print_done(Job *job);
void print_jobs();
void wait_exec(Job *job);
void fg();
void bg();

#endif