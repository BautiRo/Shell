#include "exec.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"
//
static void
get_environ_key(char *arg, char *key)
{
	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"
//
static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here
static void
set_environ_vars(char **eargv, int eargc)
{
	for (int i = 0; i < eargc; i++) {
		int idx = block_contains(eargv[i], '=');
		char key[idx], value[strlen(eargv[i]) - idx];

		get_environ_value(eargv[i], value, idx);
		get_environ_key(eargv[i], key);

		if (setenv(key, value, 1) == -1) {
			perror("setenv failed\n");
			_exit(-1);
		}
	}
}

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file
static int
open_redir_fd(char *file, int flags)
{
	int fd;
	if (flags & O_CREAT) {
		fd = open(file, flags, S_IRUSR | S_IWUSR);
	} else {
		fd = open(file, flags);
	}
	return fd;
}

// executes a command - does not return
//
// Hint:
// - check how the 'cmd' structs are defined
// 	in types.h
// - casting could be a good option
void
exec_cmd(struct cmd *cmd)
{
	// To be used in the different cases
	struct execcmd *e;
	struct backcmd *b;
	struct execcmd *r;
	struct pipecmd *p;

	switch (cmd->type) {
	case EXEC:
		// spawns a command
		//
		e = (struct execcmd *) cmd;

		set_environ_vars(e->eargv, e->eargc);

		if (execvp(e->argv[0], e->argv) == -1) {
			perror("exec failed\n");
			_exit(-1);
			break;
		};
		exit(0);

	case BACK: {
		// runs a command in background
		//
		b = (struct backcmd *) cmd;

		int i = fork();
		if (i < 0) {
			perror("fork failed\n");
			_exit(-1);
			break;
		}
		if (i == 0) {
			printf_debug("[PID=%d]\n", getpid());
			exec_cmd(b->c);
			exit(0);
		}
		int status;
		waitpid(i, &status, WNOHANG);
		exit(0);
	}

	case REDIR: {
		// changes the input/output/stderr flow
		//
		// To check if a redirection has to be performed
		// verify if file name's length (in the execcmd struct)
		// is greater than zero
		//
		r = (struct execcmd *) cmd;

		int fd_out = 0, fd_in = 0, fd_err = 0;
		if (strlen(r->out_file)) {
			fd_out = open_redir_fd(r->out_file,
			                       O_WRONLY | O_CREAT | O_TRUNC |
			                               O_CLOEXEC);
			if (dup2(fd_out, STDOUT_FILENO) == -1) {
				perror("dup failed\n");
				_exit(-1);
				break;
			}
			if (strlen(r->err_file) && strcmp(r->err_file, "&1") == 0) {
				if (dup2(fd_out, STDERR_FILENO) == -1) {
					perror("dup failed\n");
					_exit(-1);
					break;
				}
			}
			close(fd_out);
		}
		if (strlen(r->in_file)) {
			fd_in = open_redir_fd(r->in_file, O_RDONLY | O_CLOEXEC);
			if (dup2(fd_in, STDIN_FILENO) == -1) {
				perror("dup failed\n");
				_exit(-1);
				break;
			}
			close(fd_in);
		}
		if (strlen(r->err_file) && strcmp(r->err_file, "&1") != 0) {
			fd_err = open_redir_fd(r->err_file,
			                       O_WRONLY | O_CREAT | O_TRUNC |
			                               O_CLOEXEC);
			if (dup2(fd_err, STDERR_FILENO) == -1) {
				perror("dup failed\n");
				_exit(-1);
				break;
			}
			close(fd_err);
		}
		if (execvp(r->argv[0], r->argv) == -1) {
			perror("exec failed\n");
			_exit(-1);
			break;
		}
		exit(0);
	}

	case PIPE: {
		// pipes two commands
		//
		p = (struct pipecmd *) cmd;

		int fds[2];
		int r = pipe(fds);
		if (r < 0) {
			perror("pipe failed\n");
			_exit(-1);
			break;
		}
		int i = fork();
		if (i < 0) {
			perror("fork failed\n");
			close(fds[0]);
			close(fds[1]);
			_exit(-1);
			break;
		}
		if (i == 0) {
			// el hijo izquierdo no va a leer.
			close(fds[0]);

			if (dup2(fds[1], STDOUT_FILENO) == -1) {
				perror("dup failed\n");
				_exit(-1);
				break;
			}
			if (dup2(fds[1], STDERR_FILENO) == -1) {
				perror("dup failed\n");
				_exit(-1);
				break;
			}
			close(fds[1]);
			exec_cmd(p->leftcmd);
			exit(0);
		} else {
			int j = fork();
			close(fds[1]);
			if (j == 0) {
				// el hijo derecho no va a escribir.
				if (dup2(fds[0], STDIN_FILENO) == -1) {
					perror("dup failed\n");
					_exit(-1);
					break;
				}
				close(fds[0]);
				exec_cmd(p->rightcmd);
				exit(0);
			}
			close(fds[0]);
		}
		while (wait(NULL) > 0) {
		}
		free_command(parsed_pipe);
		// free the memory allocated
		// for the pipe tree structure
		exit(0);
	}
	}
}
