#define _GNU_SOURCE
#ifndef NARGS
#define NARGS 4
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
// static void execute_command(char *argvs) {
// 	if (execvp( argvs[0], argvs) == -1 ){
// 		perror("exec failed");
// 	}
// }

int
main(int argc, char *argv[])
{
	char *cmd;
	cmd = argv[1];

	size_t len[NARGS + 2] = {};
	char *argvs[NARGS + 2];
	argvs[0] = cmd;
	for (int k = 1; k <= NARGS; ++k) {
		argvs[k] = NULL;
	}
	argvs[NARGS + 1] = NULL;
	int count = 1;

	while (getline(&argvs[count], &len[count], stdin) > 0) {
		argvs[count][strcspn(argvs[count], "\n")] = 0;
		if (count == NARGS) {
			int i = fork();
			if (i < 0) {
				perror("error en fork");
				_exit(-1);
			}
			if (i == 0) {
				if (execvp(argvs[0], argvs) == -1) {
					perror("exec failed");
					_exit(-1);
				}
				exit(0);
			}
			wait(NULL);
			count = 0;
			for (int j = 1; j <= NARGS; ++j) {
				argvs[j] = NULL;
				free(argvs[j]);
			}
		}
		count++;
	}
	if (count >= 2) {
		argvs[count] = NULL;
		if (execvp(argvs[0], argvs) == -1) {
			perror("exec failed");
			_exit(-1);
		}
	}

	wait(NULL);
	exit(0);
}