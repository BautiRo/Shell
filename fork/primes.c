#include <sys/wait.h>
#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

static void
recursiveFilter(int rc)
{
	int prime = 0;
	if (read(rc, &prime, sizeof(prime)) < 0) {
		perror("error en read");
		_exit(-1);
	}
	if (prime == -1) {
		close(rc);
		exit(0);
	}
	printf("primo %d\n", prime);
	int fds[2];
	int r = pipe(fds);
	int i = fork();
	if (r < 0) {
		perror("Error en pipe");
		_exit(-1);
	}
	if (i < 0) {
		perror("error en fork");
		_exit(-1);
	} else if (i == 0) {
		// El hijo no va a escribir
		close(fds[1]);
		// Tampoco va a leer del abuelo
		close(rc);
		recursiveFilter(fds[0]);
	} else {
		// El padre no va a leer
		close(fds[0]);
		int recv = 0;
		while (1) {
			if (read(rc, &recv, sizeof(recv)) > 0) {
				if (recv == -1) {
					if (write(fds[1], &recv, sizeof(recv)) <
					    0) {
						perror("error en write");
						_exit(-1);
					}
					break;
				}
				if (recv % prime != 0) {
					if (write(fds[1], &recv, sizeof(recv)) <
					    0) {
						perror("error en write");
						_exit(-1);
					}
				}
			}
		}
		close(fds[1]);
		close(rc);
		wait(NULL);
		exit(0);
	}
}

int
main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("Ha olvidado ingresar el limite.\n");
		exit(1);
	}
	int n = atoi(argv[1]);
	int fds[2];
	int r;

	r = pipe(fds);
	if (r < 0) {
		perror("error en pipe");
		_exit(-1);
	}

	int i = fork();
	if (i < 0) {
		perror("error en fork");
		_exit(-1);
	} else if (i == 0) {
		// El hijo no va a escribir
		close(fds[1]);
		recursiveFilter(fds[0]);
	} else {
		// El padre no va a leer
		close(fds[0]);
		for (int i = 2; i <= n; i++) {
			if (write(fds[1], &i, sizeof(i)) < 0) {
				perror("error en write");
				_exit(-1);
			}
			if (i == n) {
				int last = -1;
				if (write(fds[1], &last, sizeof(last)) < 0) {
					perror("error en write");
					_exit(-1);
				}
			}
		}
		close(fds[1]);
		wait(NULL);
		exit(0);
	}
}