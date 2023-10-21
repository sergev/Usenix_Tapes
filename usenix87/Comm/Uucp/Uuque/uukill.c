/*
 * uukill
 * uucp job killer
 * s3 version (in C) by Brandon Allbery
 */

#include <stdio.h>
#include "dir.h"

main(argc, argv)
char **argv; {
	int fndjob, jkstat;

	if (argc < 2) {
		fprintf(stderr, "usage: uukill job ...\n");
		exit(1);
	}
	if (chdir("/usr/spool/uucp") < 0) {
		perror("/usr/spool/uucp");
		exit(3);
	}
	fndjob = 0;
	for (argc--, argv++; argc > 0; argc--, argv++)
		switch (uukill(*argv)) {
			case -1:
				fndjob = 2;
				break;
			case 0:
				fprintf(stderr, "uukill: no such job: %s\n", *argv);
				if (fndjob < 2)
					fndjob = 1;
				continue;
		}
	exit(fndjob);
}

uukill(jobid)
char *jobid; {
	DIR *spool;
	struct direct *job;
	char jid[8];
	int cnt;
	
	if ((spool = opendir(".")) == NULL) {
		perror("uukill: cannot open spool directory");
		exit(3);
	}
	while ((job = readdir(spool)) != NULL) {
		if (job->d_name[0] != 'C' || job->d_name[1] != '.')
			continue;
		strcpy(jid, &job->d_name[strlen(job->d_name) - 4]);
		if (strcmp(jobid, jid) != 0)
			continue;
		return koutq(job, jid);
	}
	closedir(spool);
	return 0;
}

koutq(job, jid)
char *jid;
struct direct *job; {
	FILE *jfile;
	char cmd[2], xline[512], xtmp[128], otmp[128], utmp[18];
	
	if ((jfile = fopen(job->d_name, "r")) < 0) {
		fprintf(stderr, "uukill: cannot kill %s: ", jid);
		perror("");
		return -1;
	}
	while (fgets(xline, sizeof xline, jfile) != NULL) {
		sscanf(xline, "%s %s %s %s", cmd, xtmp, otmp, utmp);
		switch (cmd[0]) {
			case 'S':
				if (!okkill(utmp)) {
					fprintf(stderr, "uukill: job %s, permission denied\n", jid);
					return 0;
				}
				if (xtmp[0] == 'D' && xtmp[1] == '.')
					if (unlink(xtmp) < 0)
						return 0;
				break;
			case 'R':
				if (!okkill(utmp)) {
					fprintf(stderr, "uukill: job %s, permission denied\n", jid);
					return 0;
				}
			default:
				fprintf(stderr, "uukill: invalid cmd %s: %s", job->d_name, xline);
				return 0;
		}
	}
	return unlink(job->d_name) == 0;
}

okkill(uid)
char *uid; {
	if (getuid() == 0)
		return 1;
	return strcmp(uid, cuserid(NULL)) == 0;
}
