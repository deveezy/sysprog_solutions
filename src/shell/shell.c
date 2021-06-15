// TODO: run program in new process
#include "shell.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/wait.h>

const char whitespaces[] = " \t\r\n\v";
const char     symbols[] = "<|>";

int 
scan_cmd(char *str, int len) 
{
	if (isatty(fileno(stdin)))	
		fprintf(stdout, "shell $: ");
	memset(str, 0, len);
	fgets(str, len, stdin);
	if (str[0] == 0) // EOF
		return -1;

	return 0;
}

command * 
parse_command(char *str) 
{
	command *cmd;
	char *str_end = str + strlen(str);
	cmd = parse_line(&str, str_end);

	peek(&str, str_end, ""); // verify full parse
  if(str != str_end){
    fprintf(stderr, "Symbols remained: %s\n", str_end);
    exit(-1);
  }
	return cmd;
}

command *
parse_line(char **ps, char *es)
{
  command *cmd;
  cmd = parse_pipe(ps, es);
  return cmd;
}


void 
run_cmd(command *cmd) 
{
	if (cmd == 0)	
		exit(128);
	command_exec *cmde;
	command_pipe *cmdp;
	command_redirect *cmdr;
	int fd[2];
	switch (cmd->type) {
		case CMD_EXEC:
			cmde = (command_exec *)cmd;
	
			if(cmde->argv[0] == 0)
				exit(0);
  
			char str1[100] = "/bin/";
			char str2[100] = "/usr/bin/";
	
			// execvp(cmde->argv[0], );
			execv(cmde->argv[0], cmde->argv); 
			execv(strcat(str1, cmde->argv[0]), cmde->argv); 
			execv(strcat(str2, cmde->argv[0]), cmde->argv);
			fprintf(stderr, "exec %s failed\n", cmde->argv[0]);
			break;

		case CMD_PIPE:
			cmdp = (command_pipe *)cmd;
			if (pipe(fd) < 0)
				exit(100);

			if (fork() == 0) { // left cmd
				if (fd[1] != STDOUT_FILENO) {
					dup2(fd[1], STDOUT_FILENO);
					close(fd[1]);
				}
				close(fd[0]);
				run_cmd(cmdp->left);
			}
			
			if (fork() == 0) { // right cmd
				if (fd[0] != STDIN_FILENO) {
					dup2(fd[0], STDIN_FILENO);
					close(fd[0]);
				}
				close(fd[1]);

				run_cmd(cmdp->right);
			}
			close(fd[0]);
			close(fd[1]);
			wait(NULL);
			wait(NULL);
			
			break;

		case CMD_REDIRECT:
			cmdr = (command_redirect *)cmd;
			close(cmdr->fd); // close input or output (free descriptor number)
	
			if (open(cmdr->file, cmdr->mode, S_IRWXU) < 0) {  // open takes the lower descriptor number
				fprintf(stderr, "open %s failed\n", cmdr->file);
			}
			run_cmd(cmdr->cmd);
			break;
		default:
			fprintf(stderr, "Unknown cmd type\n");
	}
}

command * 
command_create_exec(void)
{
	command_exec *cmd = malloc(sizeof(*cmd));
	memset(cmd, 0, sizeof(*cmd));
	cmd->type = CMD_EXEC;
	return (command *)cmd;
}

command *
command_create_pipe(command *left, command *right)
{
	command_pipe *cmd = malloc(sizeof(*cmd));
	memset(cmd, 0, sizeof(*cmd));
	cmd->type = CMD_PIPE;
	cmd->left = left;
	cmd->right = right;
	return (command *)cmd;
}

command *
command_create_redirect(command *subcmd, char *file, TOKEN_TYPE mode)
{
	command_redirect *cmd = malloc(sizeof(*cmd));
	memset(cmd, 0, sizeof(*cmd));
	cmd->type = CMD_REDIRECT;
	cmd->cmd = subcmd;
	cmd->file = file;
	cmd->mode = (mode == TOKEN_INPUT) ?  O_RDONLY : O_WRONLY | O_CREAT | O_TRUNC;
  cmd->fd = (mode == TOKEN_INPUT) ? 0 : 1;
	return (command *)cmd;
}

// parser
command *
parse_exec(char **pp_str, char *end_str)
{
	char *out_str, *out_str_end;
  TOKEN_TYPE tok;
  int argc;
	command_exec *cmde;
  command *cmd;
  
  cmd = command_create_exec();
  argc = 0;
  cmd = parse_redirects(cmd, pp_str, end_str);
  cmde = (command_exec*)cmd;
  while(!peek(pp_str, end_str, "|")) {
    if((tok = gettoken(pp_str, end_str, &out_str, &out_str_end)) == TOKEN_INVALID)
      break;
    if(tok != TOKEN_LETTER) {
      fprintf(stderr, "syntax error\n");
      exit(-1);
    }
    cmde->argv[argc] = mkcopy(out_str, out_str_end);
    argc++;
    if(argc >= MAXARGS) {
      fprintf(stderr, "too many args\n");
      exit(-1);
    }
		cmd = parse_redirects(cmd, pp_str, end_str);
  }
  cmde->argv[argc] = NULL;
  return cmd;
}

command *
parse_pipe(char **pp_str, char *end_str)
{
  command *cmd;

  cmd = parse_exec(pp_str, end_str);
  if(peek(pp_str, end_str, "|")) {
    gettoken(pp_str, end_str, 0, 0);
    cmd = command_create_pipe(cmd, parse_pipe(pp_str, end_str));
  }
  return cmd;
}

command *
parse_redirects(command *cmd, char **pp_str, char *end_str) 
{
	char *out_str, *out_str_end;
  TOKEN_TYPE tok;
	while(peek(pp_str, end_str, "<>")) {
		tok = gettoken(pp_str, end_str, NULL, NULL);
    if(gettoken(pp_str, end_str, &out_str, &out_str_end) == TOKEN_INVALID) {
      fprintf(stderr, "no redirection arg\n");
      exit(-1);
    }
		switch (tok) {
			case TOKEN_INPUT:
				cmd = command_create_redirect(cmd, mkcopy(out_str, out_str_end), TOKEN_INPUT);
				break;
			case TOKEN_OUTPUT:
				cmd = command_create_redirect(cmd, mkcopy(out_str, out_str_end), TOKEN_OUTPUT);
				break;
			default:
				fprintf(stderr, "Unknown tok type\n");
		}
	}
	return cmd;
}

TOKEN_TYPE
gettoken(char **pp_str, char *end_str, char **out_str, char **end_out_str)
{
  // for example: ptr = "  ps aux | grep audio"
  char *ptr = *pp_str;
  while (ptr < end_str && strchr(whitespaces, *ptr)) // skip whitespaces
    ++ptr; // move cursor
  // ptr = "ps aux | grep audio"

  if (out_str) // remember position to out_str: "ps aux | grep audio"
    *out_str = ptr; 

  TOKEN_TYPE tok_type = TOKEN_INVALID;
  switch(*ptr) {
  case 0:
    break;
  case '|':
		tok_type = TOKEN_PIPE;
		ptr++;
		break;
  case '<':
		tok_type = TOKEN_INPUT;
    ptr++;
    break;
  case '>':
		tok_type = TOKEN_OUTPUT;
    ptr++;
    break;
  default:
    tok_type = TOKEN_LETTER;
    while(ptr < end_str && !strchr(whitespaces, *ptr) && !strchr(symbols, *ptr))
      ptr++; // move cursor while ptr != end_str and *ptr != any whitespaces or symbols
      // i.e get one word
      // now ptr = " aux | grep audio" 
    break;
  }
  if(end_out_str)
    *end_out_str = ptr; 
  
  while(ptr < end_str && strchr(whitespaces, *ptr)) // skip whites spaces
    ptr++;  // ptr = "aux | grep audio"
  *pp_str = ptr; // ready for next parse iteration
  // range from out_str to end_out_str contains "ps" single word
  return tok_type;
}

// skip whitespaces in original string.
int
peek(char **ps, char *es, char *toks) // mutable
{
  char *s;
  
  s = *ps;
  // while s != end_of_st AND *s == any whitespace from whitespaces
  while(s < es && strchr(whitespaces, *s)) // skip whitespaces
    s++; // move cursor forward
  *ps = s;
  return *s && strchr(toks, *s); // if is_symbol(*s) AND *s == any tok from toks
}


char *
mkcopy(char *s, char *es)
{
  int n = es - s;
  char *c = malloc(n+1);
  assert(c);
  strncpy(c, s, n);
  c[n] = 0;
  return c;
}
