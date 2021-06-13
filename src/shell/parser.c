#include "parser.h"
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned long size_t;

/**
 * @brief Create new string of the str_len length and plus one extra byte 0
 *
 * @param str_len - length of the new string w/o null-terminator extra byte
 * @param s - source string
 * @return char* - null-terminated string
 */
static char * 
string_create(size_t str_len, const char *const s) {
  char *str = malloc((str_len + 1) * sizeof(char));
  assert(str != NULL);
  if (str == NULL) {
    printf("%s\n", strerror(errno));
  }
  str[str_len] = 0;
  strncpy(str, s, str_len);
  return str;
}

void 
parse_cmd_name(const char *cmd, command_struct *cmd_struct) {
  const char *ret_cmd;
  const char *ptr = cmd;
  for (size_t symbols = 0; *ptr; ++ptr, ++symbols) {
    if (*ptr == ' ') {
      ret_cmd = string_create(symbols, cmd);
      break;
    }
  }
  cmd_struct->name = ret_cmd;
}

void 
parse_cmd_args(const char *cmd, command_struct *cmd_struct) {
  assert(cmd_struct->name != NULL);
  const char *ptr = cmd;
  int argc = 0;
  char *quoted;
  if (parse_cmd_args_quoted(cmd, &quoted)) {
    cmd_struct->argv = malloc(sizeof(char *));
		cmd_struct->argc = 0;
		cmd_struct->argv[cmd_struct->argc] = quoted;
		++cmd_struct->argc;
		assert(quoted != NULL);
  } else {
    for (; *ptr; ++ptr) {
      if (*(ptr + 1) == '\0') {
        ++argc;
        break;
      } else if (*ptr == ' ') {
        ++argc;
        ++ptr;
      }
    }
    cmd_struct->argc = argc;
    cmd_struct->argv = malloc(argc * sizeof(char *));
#define CURRENT_ARG cmd_struct->argv[(argc - argc + argno++) % argc]
    ptr = cmd;
    int argno = 0;
    for (size_t symbols = 0; *ptr; ++ptr, ++symbols) {
      if (*ptr == ' ') {
        CURRENT_ARG = string_create(symbols, cmd);
        ++ptr;
        cmd += symbols + 1;
        symbols = 0;
      } else if (*(ptr + 1) == '\0') {
        ++symbols;
        CURRENT_ARG = string_create(symbols, cmd);
        ++ptr;
        cmd += symbols + 1;
        break;
      }
    }
  }
}

bool 
parse_cmd_args_quoted(const char *cmd, char **out_str) {
  const char *start = NULL;
  const char *end = NULL;
  const char *ptr = cmd;
  bool found = false;
  for (; *ptr; ++ptr) {
    if (*ptr == '\"') {
      start = ptr;
      ++ptr;
      for (; *ptr++;) {
        if (*ptr == '\"') {
          end = ptr;
          found = true;
          break;
        }
      }
      break;
    }
  }
  if (found) {
    *out_str = string_create(end + 1 - start, start);
    return true;
  }
  return false;
}

// cmd: gcc -O3 main.c -o program
void 
parse_cmd(const char *cmd) 
{
  command_struct cmd_struct;
  const char *ptr = cmd;

  // get name of cmd
  parse_cmd_name(ptr, &cmd_struct);
	parse_cmd_args(ptr, &cmd_struct);
}
