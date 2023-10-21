#include <stdio.h>
#include <errno.h>

extern int errno;
static int __pop_pid = -1;

FILE *efopen(file, mode)
char *file, *mode; {
	int alevel, chmg;
	FILE *fp;
	
	switch (mode[0]) {
		case 'r':
			alevel = 4;
			break;
		case 'w':
		case 'a':
			alevel = 2;
			break;
	}
	if (mode[1] == '+')
		alevel = 6;
	chmg = 0;
	if (access(file, alevel) < 0)
		if (errno != ENOENT)
			return (FILE *) 0;
		else
			chmg = 1;
	if ((fp = fopen(file, mode)) == (FILE *) 0)
		return (FILE *) 0;
	if (chmg)
		chown(file, getuid(), getgid());
	return fp;
}

eopen(file, mode)
char *file; {
	int alevel, fd, chmg;
	
	switch (mode & 3) {
		case 0:
			alevel = 4;
			break;
		case 1:
			alevel = 2;
			break;
		case 2:
			alevel = 6;
			break;
	}
	chmg = 0;
	if (access(file, alevel) < 0)
		if (errno != ENOENT)
			return -1;
		else
			chmg = 1;
	if ((fd = open(file, mode)) == -1)
		return -1;
	if (chmg)
		chown(file, getuid(), getgid());
	return fd;
}
		
FILE *epopen(cmd, mode)
char *mode; {
	FILE *pfp;
	int pfd[2];
	int pmode;
	
	pmode = (*mode == 'r'? 1: 0);
	pipe(pfd);
	switch (__pop_pid = fork()) {
		case -1:
			return (FILE *) 0;
		case 0:
			setgid(getgid());
			setuid(getuid());
			close(pmode);
			dup(pfd[pmode]);
			close(pfd[!pmode]);
			close(pfd[pmode]);
			execl("/bin/sh", "sh", "-c", cmd, (char *) 0);
			_exit(100);
		default:
			close(pfd[pmode]);
			return fdopen(pfd[!pmode], mode);
	}
}

pclose(fp)
FILE *fp; {
	int status;

	fclose(fp);
	if (__pop_pid == -1)
		return -1;
	while (wait(&status) != __pop_pid)
		;
	return status;
}
