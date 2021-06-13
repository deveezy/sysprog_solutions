#include "parser.h"
// #include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

void 
ASSERT_STRING_EQUALS(const char *orig, const char *expected)
{
	if (strcmp(orig, expected) == 0) {
		printf("Correct\n");
		return;
	} else {
		fprintf(stderr, "Fail: expected: %s, but passed: %s", expected, orig);
		return;
	}
}

void 
ASSERT_NUMBER_EQUALS(int orig, int expected)
{
	if (orig == expected) {
		printf("Correct\n");
		return;
	} else {
		fprintf(stderr, "Fail: expected: %d, but passed: %d\n", expected, orig);
		return;
	}
}

void
TEST_PARSE_CMD_NAME()
{
	command_struct cmd_struct1;
	command_struct cmd_struct2;
	const char *cmd1 = "ps aux";
	const char *cmd2 = "ls -lah";
	parse_cmd_name(cmd1, &cmd_struct1);
	parse_cmd_name(cmd2, &cmd_struct2);
	ASSERT_STRING_EQUALS(cmd_struct1.name, "ps");
	ASSERT_STRING_EQUALS(cmd_struct2.name, "ls");
}

void
TEST_PARSE_CMD_ARGS_QUOTED()
{
	const char *str = "grep \"quouted string\"";
	command_struct cmd_struct;
	char *res_str; 
	bool res = parse_cmd_args_quoted(str, &res_str);
	ASSERT_NUMBER_EQUALS(res, 1);
	ASSERT_STRING_EQUALS(res_str, "\"quouted string\"");
}

void
TEST_PARSE_CMD_ARGS()
{
	command_struct cmd_struct1;
	command_struct cmd_struct2;
	command_struct cmd_struct3;
	const char *cmd1 = "ps aux";
	const char *cmd2 = "ls -lah";
	const char *cmd3 = "grep ";
	parse_cmd_name(cmd1, &cmd_struct1);
	parse_cmd_name(cmd2, &cmd_struct2);
	parse_cmd_name(cmd3, &cmd_struct3);

	const char *args1 = "aux ejf";
	const char *args2 = "-lah ol lh";
	const char *args3 = "\"exe file\"";
	parse_cmd_args(args1, &cmd_struct1);
	parse_cmd_args(args2, &cmd_struct2);
	parse_cmd_args(args3, &cmd_struct3);

	ASSERT_NUMBER_EQUALS(cmd_struct1.argc, 2);
	ASSERT_STRING_EQUALS(cmd_struct1.argv[0], "aux");
	ASSERT_STRING_EQUALS(cmd_struct1.argv[1], "ejf");

	ASSERT_NUMBER_EQUALS(cmd_struct2.argc, 3);
	ASSERT_STRING_EQUALS(cmd_struct2.argv[0], "-lah");
	ASSERT_STRING_EQUALS(cmd_struct2.argv[1], "ol");
	ASSERT_STRING_EQUALS(cmd_struct2.argv[2], "lh");

	ASSERT_NUMBER_EQUALS(cmd_struct3.argc, 1);
	ASSERT_STRING_EQUALS(cmd_struct3.argv[0], "\"exe file\"");
}

int main()
{
	TEST_PARSE_CMD_NAME();
	TEST_PARSE_CMD_ARGS_QUOTED();
	TEST_PARSE_CMD_ARGS();
	return 0;
}