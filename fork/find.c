#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/limits.h>

static void
aux(DIR *dp, char *(*f)(const char *, const char *), char *search, char *path)
{
	int fd = dirfd(dp);
	struct dirent *dirp;
	while ((dirp = readdir(dp)) != NULL) {
		if ((*f)(dirp->d_name, search) != NULL) {
			char auxPath[PATH_MAX] = "";
			strcat(auxPath, path);
			strcat(auxPath, dirp->d_name);
			printf("%s\n", auxPath);
		}
		if (strcmp(dirp->d_name, ".") == 0 ||
		    strcmp(dirp->d_name, "..") == 0) {
			continue;
		}
		if (dirp->d_type == DT_DIR) {
			char auxPath[PATH_MAX] = "";
			strcat(auxPath, path);
			strcat(auxPath, dirp->d_name);
			strcat(auxPath, "/");
			int newfd = openat(fd, dirp->d_name, O_DIRECTORY);
			DIR *subdp = fdopendir(newfd);
			aux(subdp, (*f), search, auxPath);
		}
	}
	closedir(dp);
}

int
main(int argc, char *argv[])
{
	char *search;
	bool ks = true;
	if (argc < 2) {
		printf("Ha olvidado ingresar la cadena a buscar.\n");
		exit(1);
	}
	if (argc == 2) {
		search = argv[1];
	}
	if (argc > 2) {
		if (strcmp(argv[1], "-i") == 0) {
			ks = false;
			search = argv[2];
		} else {
			printf("%s\n", argv[1]);
			printf("%s\n", argv[2]);
			printf("Formato invalido.\n");
			exit(1);
		}
	}

	char *(*f)(const char *, const char *) = ks ? strstr : strcasestr;

	DIR *dp = opendir(".");
	char path[PATH_MAX] = "";
	aux(dp, (*f), search, path);

	return 0;
}