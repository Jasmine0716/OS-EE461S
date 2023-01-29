#include<stdio.h>
#include <stdlib.h>
#include<readline/readline.h>
#include<string.h>
#include<stdbool.h>
#include<fcntl.h>
#include<unistd.h>
#include<signal.h>
#include <errno.h>


#include "token.h"
#include "job.h"

Job *job_head;
Job *job_tail;

void free_job(Job *job){
	if(job->prev_job != NULL) job->prev_job->next_job = job->next_job;
	if(job->next_job != NULL) job->next_job->prev_job = job->prev_job;
	else job_tail = job->prev_job;
	free(job);
}

Job* create_job(pid_t pgid, int status, char* input){
	Job *job = (Job *)malloc(sizeof(Job));
	job->pg_id = pgid;
	job->prev_job = job_tail;
	job->next_job = NULL;

	job->job_id = 0;
	if(job->prev_job != NULL){
		job->job_id = job->prev_job->job_id+1;
		job->prev_job->next_job = job;
	}

	job->in_bg = false;
	job->in_fg = false;
	job->in_stop = false;

	job->input_cmd = input;
	
	switch(status){
		case 0:
			break; 
		case 1:
			job->in_fg = true;
			break;
		case 2:
			job->in_bg = true;
			break;
		case 3:
			job->in_stop = true;
			break;
	}

	job_tail = job;

	return job;
}

void print_jobs(){
	Job *curr_job = job_head->next_job;
	while(curr_job != NULL){
		printf("[%d] ", curr_job->job_id);
		if(curr_job->in_fg)
			printf("+");
		if(curr_job->in_bg)
			printf("-");
		if(curr_job->in_stop)
			printf(" Stopped");
		else
			printf(" Running");
		printf("\t%s \n", curr_job->input_cmd);
		curr_job = curr_job->next_job;
	}
}

void wait_exec(Job *job){
	int status;
	printf("0");
	waitpid(job->pg_id, &status, WUNTRACED);
	printf("1");
	if(WIFEXITED(status) != 0){
		printf("2");
		free_job(job);
	}else if(WIFSTOPPED(status) != 0){
		// CTRL Z
		printf("3");
		job->in_stop = true;
		job->in_fg = false;
		job->in_bg = false;
	}else if(WIFSIGNALED(status) != 0){
		// CTRL C
		printf("4");
		free_job(job);
	}
	printf("5");
	tcsetpgrp(STDIN_FILENO, getpid());
	printf("6");
}

void fg(){
	Job *curr_job = job_head->next_job;
	while(curr_job != NULL){
		if(curr_job->in_stop || curr_job->in_bg){
			curr_job->in_fg = true;
			curr_job->in_stop = false;
			curr_job->in_bg = false;
			tcsetpgrp(STDIN_FILENO, curr_job->pg_id);
			kill(-curr_job->pg_id, SIGCONT);
			wait_exec(curr_job);
			break;
		}
		curr_job = curr_job->next_job;
	}
}

void bg(int job_id){
	// 1. change status to bg
	// 2. make the terminal take control
	//
	Job *curr_job = job_head->next_job;
	while(curr_job != NULL){
		if(curr_job->in_stop){
			curr_job->in_fg = false;
			curr_job->in_stop = false;
			curr_job->in_bg = true;
			kill(-curr_job->pg_id, SIGCONT);
			break;
		}
	}
}