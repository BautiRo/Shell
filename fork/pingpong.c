#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int
main()
{
	int fds1[2];
	int fds2[2];
	int r1;
	int r2;

	r1 = pipe(fds1);
	r2 = pipe(fds2);
	if (r1 < 0 || r2 < 0) {
		perror("error en pipe");
		_exit(-1);
	}

	printf("hola soy pid %d\n", getpid());
	printf("-primer pipe me devuelve [%d, %d]\n", fds1[0], fds1[1]);
	printf("-segundo pipe me devuelve [%d, %d]\n\n", fds2[0], fds2[1]);

	int i = fork();
	if (i < 0) {
		perror("error en fork");
		_exit(-1);
	} else if (i == 0) {
		// El hijo no va a escribir en el primer pipe
		close(fds1[1]);
		// Tampoco va a leer del segundo pipe
		close(fds2[0]);

		int recv = 0;
		if (read(fds1[0], &recv, sizeof(recv)) < 0) {
			perror("[hijo] error en read");
			_exit(-1);
		}
		if (write(fds2[1], &recv, sizeof(recv)) < 0) {
			perror("[hijo] error en write");
			_exit(-1);
		}

		printf("donde fork me devuelve 0\n");
		printf("-getpid me devuelve %d\n", getpid());
		printf("-getppid me devuelve %d\n", getppid());
		printf("-recibo valor %d via fd=%d\n", recv, fds1[0]);
		printf("-reenvio valor en fd=%d y termino\n\n", fds2[1]);

		close(fds1[0]);
		close(fds2[1]);
	} else {
		// El padre no va a leer del primer pipe
		close(fds1[0]);
		// Tampoco va a escribir en el segundo pipe
		close(fds2[1]);

		srand(time(NULL));
		int msg = random();

		if (write(fds1[1], &msg, sizeof(msg)) < 0) {
			perror("[padre] error en write");
			_exit(-1);
		}

		printf("donde fork me devuelve %d\n", i);
		printf("-getpid me devuelve %d\n", getpid());
		printf("-getppid me devuelve %d\n", getppid());
		printf("-random me devuelve %d\n", msg);
		printf("-envio valor %d a traves de fd=%d\n\n", msg, fds1[1]);

		// Espero un segundo, el hijo no deberia seguir
		sleep(1);
		int recv = 0;
		if (read(fds2[0], &recv, sizeof(recv)) < 0) {
			perror("[padre] error en read");
			_exit(-1);
		}

		printf("hola de nuevo pid %d\n", getpid());
		printf("-recibi valor %d via fd=%d\n", recv, fds2[0]);

		close(fds1[1]);
		close(fds2[0]);
	}
	return 0;
}
