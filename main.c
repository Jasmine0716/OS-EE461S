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

extern Job *job_head;
extern Job *job_tail;

void signal_handler(int sig){
	if(sig == SIGCHLD){
		int status;
		Job *curr_job;
		Job *p_job = job_head->next_job;
		int cnt = 0;
		while(p_job != NULL){
			curr_job = p_job;
			p_job = curr_job->next_job;
			if(waitpid(-curr_job->pg_id, &status, WNOHANG) == 0)
				continue;
			else{
				if(WIFEXITED(status) != 0 && curr_job->in_bg){
					print_done(curr_job);
		    		free_job(curr_job);
		    	}
			}
		}
	}
}

int exec_cmd(Command* command){
	if(command->in_redirect != NULL){
		int fd = open(command->in_redirect, O_RDONLY);
		if(fd == -1){
			exit(-1);
		}
		dup2(fd, STDIN_FILENO);
		close(fd);
	}

	if(command->out_redirect != NULL){
		int fd = open(command->out_redirect, O_WRONLY|O_CREAT, S_IRWXU);
		if(fd == -1){
			exit(-1);
		}
		dup2(fd, STDOUT_FILENO);
		close(fd);
	}

	if(command->error_redirect != NULL){
		int fd = open(command->error_redirect, O_WRONLY|O_CREAT, S_IRWXU);
		if(fd == -1){
			exit(-1);
		}
		dup2(fd, STDERR_FILENO);
		close(fd);
	}
    return execvp(command->cmd, command->args);
}

void exec_one_cmd(Command* command){
	int cpid = fork();
	Job *job = create_job(cpid, command->is_bg_cmd? 2 : 1, command->input_cmd);
	if (cpid == 0){
		if(signal(SIGINT, SIG_DFL) == SIG_ERR){
			printf("ERROR");
		}
		if(signal(SIGTSTP, signal_handler) == SIG_ERR){
			printf("ERROR");
		}
		setpgid(getpid(), getpid()); 
		if(job->in_fg){
			tcsetpgrp(STDIN_FILENO, getpid());
		}

		if(exec_cmd(command) == -1){
			printf("error\n");
			free_job(job);
		}
	}else{
	    if(job->in_fg){
			tcsetpgrp(STDIN_FILENO, cpid);

			wait_exec(job);
	    }else{
	    	tcsetpgrp(STDIN_FILENO, getpid());
	    }
	}
}

void exec_pipe_cmd(Command* command){
	int pipefd[2];
	if(pipe(pipefd)){
		exit(-1);
	}

	int cpid_l = fork();
	Job *job = create_job(cpid_l, command[1].is_bg_cmd? 2 : 1, command->input_cmd);

	if (cpid_l == 0){ // child process 1: on the left side of the pipe
		if(signal(SIGINT, SIG_DFL) == SIG_ERR){
			printf("ERROR");
		}
		if(signal(SIGTSTP, signal_handler) == SIG_ERR){
			printf("ERROR");
		}

		signal(SIGTTOU, SIG_IGN);

		setpgid(getpid(), getpid());
		printf("set left id\n");
		if(job->in_fg){
			printf("get terminal control\n");
			tcsetpgrp(STDIN_FILENO, getpid());
			printf("get terminal control\n");
		}
		printf("111\n");
		close(pipefd[0]);
		printf("2\n");
		dup2(pipefd[1], STDOUT_FILENO);
		printf("3\n");
		if(exec_cmd(command) == -1){
			printf("error\n");
			free_job(job);
		}
	}else{
		printf("4\n");
		int cpid_r = fork();
		printf("5\n");
		if(cpid_r == 0){
			if(signal(SIGINT, SIG_DFL) == SIG_ERR){
				printf("ERROR");
			}
			if(signal(SIGTSTP, signal_handler) == SIG_ERR){
				printf("ERROR");
			}
			
			signal(SIGTTOU, SIG_IGN);
			printf("adad %d\n", setpgid(getpid(), cpid_l)); 
			printf("set right id\n");
			close(pipefd[1]);
			printf("Done");
			dup2(pipefd[0], STDIN_FILENO);
			printf("Done");
			if(exec_cmd(&command[1]) == -1){
				printf("error\n");
				free_job(job);
			}
			printf("Done");
		}else{
			printf("a\n");
			close(pipefd[0]);
			close(pipefd[1]);

			printf("a\n");
			if(job->in_fg){
				printf("a\n");
				tcsetpgrp(STDIN_FILENO, cpid_l);

				printf("a\n");
				wait_exec(job);
		    }else{
		    	tcsetpgrp(STDIN_FILENO, getpid());
		    }
		}
	}
}

int main(){
	char *input_str;
	job_head = create_job(0, 0, NULL);
	job_tail = job_head;
	// create_job()
	while(1){
		signal(SIGINT, SIG_IGN);
		signal(SIGTSTP, SIG_IGN);
		signal(SIGTTOU, SIG_IGN);
		signal(SIGCHLD, signal_handler);
		input_str = readline("# ");
		
		if(input_str == NULL){
			exit(-1);
		}

		Command *commands = parse_cmd(input_str);
		if(!commands->has_cmd){
			continue;
		}
		if(strcmp(commands->input_cmd, "jobs") == 0){
			print_jobs();
			continue;
		}
		if(strcmp(commands->input_cmd, "fg") == 0){
			fg();
			continue;
		}
		if(strcmp(commands->input_cmd, "bg") == 0){
			bg();
			continue;
		}
		if(commands->has_pipe){
			exec_pipe_cmd(commands);
		}else{
			exec_one_cmd(commands);
		}
	}
}
