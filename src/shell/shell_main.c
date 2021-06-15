#include "shell.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>

// int 
// main()
// {
// 	static char buf[100];

// 	// Read and run input commands
// 	while (scan_cmd(buf, sizeof(buf)) >= 0) {
// 		if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' '){
//       // Clumsy but will have to do for now.
//       // Chdir has no effect on the parent if run in the child.
//       buf[strlen(buf)-1] = 0;  // chop \n
//       if(chdir(buf+3) < 0)
//         fprintf(stderr, "cannot cd %s\n", buf+3);
//       continue;
//     }
// 		// TODO: fork and run program
// 		if (fork() == 0) {
// 			run_cmd(parse_command(buf));
// 			wait(NULL);
// 		}
// 	}
// 	return 0;
// }