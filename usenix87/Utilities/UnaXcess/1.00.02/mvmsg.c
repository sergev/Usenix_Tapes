/*
 *	@(#)mvmsg.c	1.1 (TDI) 2/3/87
 *	@(#)Copyright (C) 1984, 85, 86, 87 by Brandon S. Allbery.
 *
 *	@(#)This file is part of UNaXcess version 1.0.2.
 *
 *	Permission is hereby granted to copy and distribute this program
 *	freely.  Permission is NOT given to modify this program or distribute
 *	it at cost, except for charging a reasonable media/copying fee.
 */

#ifndef lint
static char _FileID_[] = "@(#)mvmsg.c	1.1 (TDI) 2/3/87";
static char _UAID_[]   = "@(#)UNaXcess version 1.0.2";
#endif lint

#include <stdio.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef SYS3
#  ifdef XENIX3
#    define RIndex(s,c) strrchr(s, c)
#    define Index(s,c) strchr(s, c)
extern char *strrchr(), *strchr();
#  else  XENIX3
#    ifdef XENIX5
#      define RIndex(s,c) strrchr(s, c)
#      define Index(s,c) strchr(s, c)
extern char *strrchr(), *strchr();
#    else  XENIX5
#      define RIndex(s,c) rindex(s,c)
#      define Index(s,c) index(s,c)
extern char *rindex(), *index();
#    endif XENIX5
#  endif XENIX3
#else
#  define RIndex(s,c) strrchr(s, c)
#  define Index(s,c) strchr(s, c)
extern char *strrchr(), *strchr();
#endif

extern long atol();
extern struct passwd *getpwuid();

main(argc, argv)
char **argv; {
	char msgdir[256], conf[33];
	long msg;
	char *cp, *dp;
	int status;

	if (argc != 3) {
		fprintf(stderr, "Usage: mvmsg conf[/n] conf\n");
		exit(1);
	}
	strcpy(msgdir, getpwuid(geteuid())->pw_dir);
	strcat(msgdir, "/msgdir");
	for (cp = argv[1], dp = conf; *cp != '/' && *cp != '\0'; cp++, dp++)
		*dp = *cp;
	*dp = '\0';
	if (*cp == '\0') {
		FILE *fp;
		char tmp[512];
		long limit, cnt;
		
		sprintf(tmp, "%s/%s/himsg", msgdir, conf);
		if ((fp = fopen(tmp, "r")) == (FILE *) 0) {
			fprintf(stderr, "Conference %s: missing himsg...\n", conf);
			exit(7);
		}
		fgets(tmp, sizeof tmp, fp);
		fclose(fp);
		if ((limit = atol(tmp)) <= 0) {
			fprintf(stderr, "Conference %s: invalid himsg...\n", conf);
			exit(8);
		}
		for (cnt = 0, msg = 1; msg <= limit; msg++)
			if ((status = mvmsg(msgdir, conf, msg, argv[2])) > 0)
				exit(status);
			else if (status == 0)
				cnt++;
		printf("%ld messages moved from %s to %s.\n", cnt, conf, argv[2]);
		sprintf(tmp, "%s/%s/himsg", msgdir, conf);
		if (unlink(tmp) != 0)
			exit(12);
		sprintf(tmp, "%s/%s", msgdir, conf);
		if (rmdir(tmp) != 0)
			exit(13);
		exit(cnt == 0? 9: 0);
	}
	msg = atol(++cp);
	if (Index(argv[2], '/') != (char *) 0) {
		fprintf(stderr, "Usage: mvmsg conf[/n] conf\n");
		exit(1);
	}
	if ((status = mvmsg(msgdir, conf, msg, argv[2])) == -1)
		fprintf(stderr, "Couldn't read %s/%ld\n", conf, msg);
	exit(status);
}

mvmsg(base, conf, msg, dest)
char *base, *conf, *dest;
long msg; {
	char path[512], temp[512];
	long newmsg;
	FILE *ifp, *ofp;
	struct stat sbuf;
	
	sprintf(path, "%s/%s/%ld", base, conf, msg);
	if (stat(path, &sbuf) < 0)
		return -1;	/* likely to be common... */
	sprintf(path, "%s/%s/himsg", base, dest);
	if ((ifp = fopen(path, "r")) == (FILE *) 0) {
		fprintf(stderr, "Conference %s: missing himsg...\n", dest);
		return 2;
	}
	fgets(temp, sizeof temp, ifp);
	fclose(ifp);
	if ((newmsg = atol(temp)) <= 0) {
		fprintf(stderr, "Conference %s: invalid himsg...\n", dest);
		return 2;
	}
	newmsg++;
	sprintf(path, "%s/%s/%ld", base, conf, msg);
	if ((ifp = fopen(path, "r")) == (FILE *) 0) {
		fprintf(stderr, "Conference %s: check permissions on message %ld\n", conf, msg);
		return 3;
	}
	sprintf(path, "%s/%s/%ld", base, dest, newmsg);
	if (stat(path, &sbuf) == 0) {
		fprintf(stderr, "Conference %s: corrupted (himsg incorrect)\n", dest);
		fclose(ifp);
		return 4;
	}
	if ((ofp = fopen(path, "w")) == (FILE *) 0) {
		fprintf(stderr, "Conference %s: check permissions (can't create message)\n", dest);
		fclose(ifp);
		return 5;
	}
	while (fgets(temp, sizeof temp, ifp) != (char *) 0) {
		fputs(temp, ofp);
		if (ferror(ofp)) {
			fprintf(stderr, "Write error on %s/%ld\n", dest, newmsg);
			exit(5);	/* fatal! */
		}
	}
	if (ferror(ifp)) {
		fprintf(stderr, "Read error on %s/%ld\n", conf, msg);
		fclose(ifp);
		fclose(ofp);
		return 6;
	}
	fclose(ifp);
	fclose(ofp);
	sprintf(path, "%s/%s/himsg", base, dest);
	if ((ifp = fopen(path, "w")) == (FILE *) 0) {
		fprintf(stderr, "Conference %s: check permissions on himsg...\n", dest);
		return 2;
	}
	fprintf(ifp, "%ld\n", newmsg);
	fclose(ifp);
	sprintf(path, "%s/%s/%ld", base, conf, msg);
	if (unlink(path) < 0)
		return 10;
	return 0;
}

#ifndef BSD

/* Berkeley has a rmdir() system call!  Heaven!  But I'm on sys3 (boo hiss) */

rmdir(path)
char *path; {
	int pid, status;
	
	switch (pid = fork()) {
		case -1:
			return -1;
		case 0:
			execl("/bin/rmdir", "rmdir", path, 0);
			exit(-100);
		default:
			while (wait(&status) != pid)
				;
			return status;
	}
}

#endif BSD
