#ifndef TOKEN_H
#define TOKEN_H

typedef struct Commands{
	char *input_cmd;
	char *cmd;
	int arg_cnt;
	char *args[2001];
	char *in_redirect;
	char *out_redirect;
	char *error_redirect;
	bool has_pipe;
	bool has_cmd;
	bool is_bg_cmd;
}Command;

Command* parse_cmd(char* input_str);
void free_cmd(Command *command);

#endif