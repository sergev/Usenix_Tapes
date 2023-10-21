#define MAXSTRING 132
#define MAXDIR 60

#include <stdio.h>

#include "ndir.h"
#include <sys/stat.h>

char newsbuf[512], archdir[MAXDIR], make_dirs[8][MAXDIR], target[MAXDIR];
char in_string[MAXSTRING];

main(argc,argv)
int argc;
char **argv;
{   char rootdir[MAXDIR], newspath[MAXDIR], newsgroup[MAXDIR];

    getcwd(rootdir, MAXDIR);
    *newspath = '\0';
    *newsgroup = '\0';
    if (argc > 1) {
	strcpy(archdir, argv[1]);
    }
    else {
	fprintf(stderr, "archnews archive_directory\n");
	exit(1);
    }
    procdir(rootdir, newspath, newsgroup, 0);
}

procdir(dirpath, newspath, newsgroup, level)
char *dirpath, *newspath, *newsgroup;
{   DIR *directory;
    FILE *fp, *fp1, *pin, *pout;
    struct direct *dp;
    struct stat buf;
    int ldirpath, lnewspath, lnewsgroup, i, f_status, child_status, pid;
    char entryname[MAXDIR], *str, c;

    if (!(directory = opendir(dirpath))) {
	fprintf(stderr, "unable to open directory %s\n", dirpath);
    }
    else {
	while (dp = readdir(directory)) {

	    if (*(dp->d_name) == '.') continue;

	    *entryname= '\0';
	    strncat(entryname, dp->d_name, dp->d_namlen);
	    fp = fopen(entryname, "r");
	    if (!fp) {
		fprintf(stderr, "unable to open file %s/%s\n",
				 dirpath, entryname);
		continue;
	    }

	    fstat(fileno(fp), &buf);
	    if (buf.st_mode & S_IFDIR) {
		fclose(fp);

		ldirpath = strlen(dirpath);
		strcat(dirpath, "/");
		strcat(dirpath, entryname);

		lnewspath = strlen(newspath);
		if (lnewspath) {
		    strcat(newspath, "/");
		}
		strcat(newspath, entryname);

		lnewsgroup = strlen(newsgroup);
		if (lnewsgroup) {
		    strcat(newsgroup, ".");
		}
		strcat(newsgroup, entryname);

		strcpy(target, archdir);
		strcat(target, "/");
		strcat(target, newspath);
		fp = fopen(target, "r");
		if (!fp) {
		    strcpy(&make_dir[level][0], target);
		}
		else {
		    fclose(fp);
		}

		chdir(entryname);

		procdir(dirpath, newspath, newsgroup, level + 1);

		make_dir[level][0] = '\0';
		dirpath[ldirpath] = '\0';
		chdir(dirpath);
		newspath[lnewspath] = '\0';
		newsgroup[lnewsgroup] = '\0';
	    }
	    else {
		for (i = 0; i < level; i++) {
		    if (make_dir[i][0]) {
			if (mkdir(&make_dir[i][0], 0777)) {
			    fprintf(stderr, "cannot create directory %s, archive full\n",
					    &make_dir[i][0]);
			}
			else {
			    make_dir[i][0] = '\0';
			}
		    }
		}

		strcpy(target, archdir);
		strcat(target, "/");
		strcat(target, newspath);
		strcat(target, "/");
		strcat(target, entryname);
		strcat(target, ".Z");
		fp1 = fopen(target, "w");
		if (!fp1) {
		    fprintf(stderr, "unable to create %s, archive full\n", target);
		    exit(1);
		}

		if (pipe_command("/usr/lib/news/compress", "compress", &pin, &pout)) {
		    fprintf(stderr, "pipe to /usr/lib/news/compress failed\n");
		}

		if (!(pid = fork())) {

		    fclose(fp);
		    fclose(pin);

		    c = getc(pout);
		    while (!feof(pout)) {
			if ((f_status = putc(c, fp1)) == EOF) {
			    break;
			}
			c = getc(pout);
		    }
		    fclose(fp1);

		    if (f_status == EOF) {
			while (!feof(pout)) getc(pout);
			fclose(pout);
			exit(1);
		    }
		    else {
			fclose(pout);
			exit(0);
		    }
		}
		else {

		    fclose(fp1);
		    fclose(pout);

		    *newsbuf = '\0';
		    str = newsbuf;
		    i = 0;
		    while (fgets(in_string, MAXSTRING - 1, fp)) {
			fputs(in_string, pin);
			if (!strncmp(in_string, "Subject", 7)) {
			    sprintf(str, "%s:%s - %s", newsgroup, entryname, in_string);
			    str += strlen(str);
			}
			else if (!strncmp(in_string, "Summary", 7)) {
			    sprintf(str, "%s:%s - %s", newsgroup, entryname, in_string);
			    str += strlen(str);
			}
			else if (!strncmp(in_string, "Keywords", 8)) {
			    sprintf(str, "%s:%s - %s", newsgroup, entryname, in_string);
			    str += strlen(str);
			}
		    }
		    fclose(fp);
		    fclose(pin);

		    while (wait(&child_status) != pid);
		    if (child_status >> 8) {
			fprintf(stderr, "write to %s failed, archive full\n",
					target);
			unlink(target);
		    }
		    else {
			fputs(newsbuf, stdout);
			unlink(entryname);
		    }
		}
	    }
	}
	closedir(directory);
    }
}

pipe_command(path, command, pin, pout)
char *path, *command;
FILE **pin;
FILE **pout;

{   int fildes1[2], fildes2[2], status = 0;

    if (pipe(fildes1)) {
	fprintf(stderr, "cannot create pipes for %s command\n", path);
	status = 1;
    }
    if (pipe(fildes2)) {
	fprintf(stderr, "cannot create pipes for %s command\n", path);
	status = 1;
    }
    if (!fork()) {
	close(fildes1[1]);
	close(fildes2[0]);
	close(0);
	dup(fildes1[0]);
	close(fildes1[0]);
	close(1);
	dup(fildes2[1]);
	close(fildes2[1]);
	(void) execlp(path, command, (char *) NULL);
	perror(path);
	_exit(1);
    }
    else {
	close(fildes1[0]);
	close(fildes2[1]);
	*pin = fdopen(fildes1[1], "w");
	*pout = fdopen(fildes2[0], "r");
	if (!(*pin) || !(*pout)) status = 1;
    }
    return(status);
}
