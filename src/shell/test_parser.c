#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "shell.h"

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

// int main()
// {
//   static char buf[100];
//   int fd, r;

//   // Read and run input commands.
//   while(scan_cmd(buf, sizeof(buf)) >= 0){
//     if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' '){
//       // Clumsy but will have to do for now.
//       // Chdir has no effect on the parent if run in the child.
//       buf[strlen(buf)-1] = 0;  // chop \n
//       if(chdir(buf+3) < 0)
//         fprintf(stderr, "cannot cd %s\n", buf+3);
//       continue;
//     }
//     if (fork() == 0) {
//       run_cmd(parse_command(buf));
//     }
//     wait(NULL);
//   }
// 	return 0;
// }