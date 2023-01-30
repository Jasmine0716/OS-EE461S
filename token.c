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

Command* parse_cmd(char* input_str){
	char *sep = " ";
	char *token, *save_ptr;
	Command *commands = (Command *)malloc(2*sizeof(Command));
	int cmd_cnt = 0;
	Command* curr_cmd = commands;
	curr_cmd->input_cmd = strdup(input_str);

	for(token = strtok_r(input_str, sep, &save_ptr); 
		token; 
		token = strtok_r(input_str, sep, &save_ptr))
	{
		if(!curr_cmd->has_cmd){
			curr_cmd->cmd = token;
			curr_cmd->args[curr_cmd->arg_cnt++] = token;
			curr_cmd->has_cmd = true;
		}else if(strcmp(token, "<") == 0 ){
			curr_cmd->in_redirect = strtok_r(NULL, sep, &save_ptr); 
		}else if(strcmp(token, ">") == 0){
			curr_cmd->out_redirect = strtok_r(NULL, sep, &save_ptr);
		}else if(strcmp(token, "2>") == 0){
			curr_cmd->error_redirect = strtok_r(NULL, sep, &save_ptr);
		}else if(strcmp(token, "|") == 0){
			curr_cmd->has_pipe = true;
			curr_cmd = &commands[1];
		}else if(strcmp(token, "&") == 0){
			curr_cmd->is_bg_cmd = true;
		}else{
			curr_cmd->args[curr_cmd->arg_cnt++] = token;
		}
		input_str = NULL;
	}
	printf("command 1: %d %s\n", commands[0].arg_cnt, commands[0].cmd);
	printf("command 2: %d %s\n", commands[1].arg_cnt, commands[2].cmd);
	return commands;
}

void free_cmd(Command *command){
	free(command);
}