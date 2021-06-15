#pragma once

#define MAXARGS 6

typedef unsigned long size_t;

typedef enum CMD_TYPE { CMD_INVALID, CMD_EXEC, CMD_PIPE, CMD_REDIRECT } CMD_TYPE;

typedef enum TOKEN_TYPE { TOKEN_INVALID, TOKEN_LETTER, TOKEN_PIPE, TOKEN_INPUT, TOKEN_OUTPUT } TOKEN_TYPE;

typedef struct command {
  CMD_TYPE type;
} command;

typedef struct command_exec {
  CMD_TYPE type;
	char *argv[MAXARGS];
} command_exec;

typedef struct command_pipe {
  CMD_TYPE type;
	command *left;
	command *right;
} command_pipe;

typedef struct command_redirect {
  CMD_TYPE type;
	command *cmd;
	char *file;
	int mode;
	int fd;
} command_redirect;

int 			scan_cmd(char *, int );
command * parse_command(char *);
void 			run_cmd(command *);

command * parse_line(char **, char *); 
command * command_create_exec();
command * command_create_pipe(command *, command *);
command * command_create_redirect(command *, char *, TOKEN_TYPE);
command * parse_exec(char **, char *);
command * parse_pipe(char **, char *);
command * parse_redirects(command *, char **, char *);

TOKEN_TYPE gettoken(char **, char *, char **, char **);
int        peek(char **, char *, char *);
char     * mkcopy(char *, char *);
