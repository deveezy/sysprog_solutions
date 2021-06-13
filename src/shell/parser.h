#include <stdbool.h>

typedef struct {
	const char *name;
	const char **argv;
	int argc;
} command_struct;

void parse_cmd_name(const char *cmd, command_struct *cmd_struct);
void parse_cmd_args(const char *cmd, command_struct *cmd_struct);
bool parse_cmd_args_quoted(const char *cmd, char **out_str);
void parse_cmd(const char *cmd);