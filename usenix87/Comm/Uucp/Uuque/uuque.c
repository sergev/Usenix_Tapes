/*
 * uuque.c
 * uucp queue lister
 * s3 version (in C) by Brandon Allbery
 */

#include <stdio.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "dir.h"

struct utsname luuname;
char qsys[8], jobsys[8], jid[8];
int verbose, jobnames, jobs;

long uuque(), outq();

main(argc, argv)
char **argv; {
	extern int optind;
	extern char *optarg;
	char optch;
	long total;

	verbose = 0;
	jobnames = 0;
	jobs = 0;
	while ((optch = getopt(argc, argv, "vs:j")) != EOF)
		switch (optch) {
			case 'v':
				verbose = !verbose;
				break;
			case 'j':
				jobnames = !jobnames;
				break;
			case 's':
				strncpy(qsys, optarg, 7);
				qsys[7] = '\0';
				break;
			case '?':
				exit(1);
		}
	if (argv[optind] != NULL) {
		fprintf(stderr, "Usage: uuque [-v] [-j] [-ssystem]\n");
		exit(1);
	}
	uname(&luuname);
	if (chdir("/usr/spool/uucp") < 0) {
		perror("/usr/spool/uucp");
		exit(2);
	}
	total = uuque('C');	/* print send queue */
	uuque('X');	/* print rcv queue */
	if (jobs == 0)
		printf("No jobs queued.\n");
	else
		printf("%s%7ld total for %d jobs\n", (jobnames? "     ": ""), total, jobs);
	exit(0);
}

long uuque(sr)
char sr; {
	DIR *spool;
	struct direct *job;
	int cnt;
	long bytes;
	
	if ((spool = opendir(".")) == NULL) {
		fprintf(stderr, "uuque: cannot open spool directory\n");
		exit(2);
	}
	bytes = 0;
	while ((job = readdir(spool)) != NULL) {
		if (job->d_name[0] != sr || job->d_name[1] != '.')
			continue;
		for (cnt = 2; cnt < strlen(job->d_name) - 5; cnt++)
			jobsys[cnt - 2] = job->d_name[cnt];
		jobsys[cnt - 2] = '\0';
		strcpy(jid, &job->d_name[strlen(job->d_name) - 4]);
		if (qsys[0] != '\0' && strcmp(jobsys, qsys) != 0)
			continue;
		if (jobs++ == 0)
			printf("%s   SIZE JOB\n", (jobnames? "ID   ": ""));
		if (sr == 'C')
			bytes += outq(job);
		else
			inq(job);
	}
	closedir(spool);
	return bytes;
}

long outq(job)
struct direct *job; {
	FILE *jfile, *cfile;
	char cmd[2], xline[512], xfile[15], dfile[15], xtmp[128], dtmp[15], otmp[128], utmp[18], from[128], ftmp[150];
	struct stat statbuf;
	int pj;
	long tsize;
	
	if ((jfile = fopen(job->d_name, "r")) < 0)
		return 0;
	xfile[0] = '\0';
	dfile[0] = '\0';
	pj = 0;
	tsize = 0;
	while (fgets(xline, sizeof xline, jfile) != NULL) {
		sscanf(xline, "%s %s %s %s", cmd, xtmp, otmp, utmp);
		switch (cmd[0]) {
			case 'S':
				sprintf(dtmp, "D.%.7sX", luuname.nodename);
				if (strncmp(xtmp, dtmp, strlen(dtmp)) == 0)
					strcpy(xfile, xtmp);
				else if (xtmp[0] == 'D' && xtmp[1] == '.')
					strcpy(dfile, xtmp);
				else {
					stat(xtmp, &statbuf);
					printf("%s%s%7ld uucp %s %s!%s (%s)\n", (jobnames? (pj? "    ": jid): ""), (jobnames? " ": ""), statbuf.st_size, xtmp, jobsys, otmp, utmp);
					tsize += statbuf.st_size;
					pj++;
				}
				break;
			case 'R':
				printf("%s%s        uucp %s!%s %s (%s)\n", (jobnames? (pj? "    ": jid): ""), (jobnames? " ": ""), jobsys, xtmp, otmp, utmp);
				pj++;
				break;
			default:
				fprintf(stderr, "uuque: invalid cmd %s: %s", job->d_name, xline);
		}
	}
	fclose(jfile);
	if (xfile[0] == '\0')
		return tsize;
	if ((jfile = fopen(xfile, "r")) == NULL)
		return tsize;
	from[0] = '\0';
	while (fgets(xline, sizeof xline, jfile) != NULL) {
		sscanf(xline, "%s %s %s %s", cmd, xtmp, otmp, utmp);
		switch (cmd[0]) {
			case 'U':
				sprintf(from, "%s!%s", otmp, xtmp);
				break;
			case 'F':
			case 'I':
			case 'Z':
				break;
			case 'C':
				stat(dfile, &statbuf);
				if (strcmp(xtmp, "rmail") == 0) {
					if ((cfile = fopen(dfile, "r")) != NULL) {
						fgets(ftmp, sizeof ftmp, cfile);
						fclose(cfile);
						sscanf(ftmp, "From %s ", from);
					}
				}
				printf("%s%s%7ld %s %s!%s (%s)\n", (jobnames? (pj? "    ": jid): ""), (jobnames? " ": ""), statbuf.st_size, xtmp, jobsys, otmp, from);
				if (verbose && (strcmp(xtmp, "rmail") == 0 || strcmp(xtmp, "rnews") == 0) && (cfile = fopen(dfile, "r")) != NULL) {
					while (fgets(xline, sizeof xline, cfile) != NULL) {
						if (strncmp(xline, "Subject: ", 9) == 0)
							printf("%s        %s", (jobnames? "     ": ""), xline);
						else if (strncmp(xline, "Newsgroups: ", 12) == 0)
							printf("%s        %s", (jobnames? "": "     "), xline);
						else if (xline[0] == '\n')
							break;
					}
					fclose(cfile);
				}
				return statbuf.st_size;
			default:
				fprintf(stderr, "uuque: invalid command %s: %s", xfile, xline);
		}
	}
}

inq(job)
struct direct *job; {
	FILE *jfile, *cfile;
	char xline[512], cmd[2], utmp[50], stmp[50], from[128], dtmp[50], dfile[50], ftmp[150], xargs[100];
	int isxqt;
	struct stat statbuf;
	int pj;
	
	pj = 0;
	isxqt = 0;
	if ((jfile = fopen(job->d_name, "r")) == NULL)
		return;
	while (fgets(xline, sizeof xline, jfile) != NULL) {
		xargs[0] = '\0';
		sscanf(xline, "%s %s %s %[^\n]", cmd, utmp, stmp, xargs);
		switch (cmd[0]) {
			case 'U':
				sprintf(from, "%s!%s", stmp, utmp);
				break;
			case 'Z':
			case 'I':
				break;
			case 'F':
				if (stat(utmp, &statbuf) == 0)
					strcpy(dfile, utmp);
				else {
					sprintf(dtmp, "/usr/lib/uucp/.XQTDIR/%s", stmp);
					if (stat(dtmp, &statbuf) == 0) {
						strcpy(dfile, dtmp);
						isxqt = 1;
					}
					else
						return;
				}
				break;
			case 'C':
				if (strcmp(utmp, "rmail") == 0)
					if ((cfile = fopen(dfile, "r")) != NULL) {
						fgets(ftmp, sizeof ftmp, cfile);
						fclose(cfile);
						sscanf(ftmp, "From %s ", from);
					}
				stat(dfile, &statbuf);
				printf("%s%s%7ld%s %s%s%s%s\n", (jobnames? (pj? "    ": jid): ""), (jobnames? " ": ""), statbuf.st_size, utmp, (xargs[0] == '\0'? "": " "), (xargs[0] == '\0'? "": xargs), (isxqt? " [Executing]": ""));
				if (verbose && (strcmp(utmp, "rmail") == 0 || strcmp(utmp, "rnews") == 0) && (cfile = fopen(dfile, "r")) != NULL) {
					while (fgets(xline, sizeof xline, cfile) != NULL) {
						if (strncmp(xline, "Subject: ", 9) == 0)
							printf("%s        %s", (jobnames? "     ": ""), xline);
						else if (strncmp(xline, "Newsgroups: ", 12) == 0)
							printf("%s        %s", (jobnames? "     ": ""), xline);
						else if (xline[0] == '\n')
							break;
					}
					fclose(cfile);
				}
				return;
			default:
				fprintf(stderr, "uuque: bad cmd in %s: %s", job->d_name, xline);
		}
	}
}
