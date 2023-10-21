h60012
s 00263/00000/00000
d D 1.1 86/07/18 21:10:36 brandon 1 0
c Converted to SCCS 07/18/86 21:10:35
e
u
U
f b 
f q 0.4.5
f t UNaXcess
t
T
I 1
/*
 *	%W% (TDI) %G% %U%
 *	%Z%Copyright (C) 1984, 85, 86 by Brandon S. Allbery.
 *
 *	%Z%This file is part of %Y% version %Q%.
 *
 *	Permission is hereby granted to copy and distribute this program
 *	freely.  Permission is NOT given to modify this program or distribute
 *	it at cost, except for charging a reasonable media/copying fee.
 */

#ifndef lint
static char _FileID_[] = "%W% (TDI) %G% %U%";
static char _UAID_[]   = "%Z%%Y% version %Q%";
#endif lint

#include <stdio.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define NEWSRC		"%newslink"	/* simplified .newsrc */
#define ACTIVE		"/usr/lib/news/active"
#define NEWSDIR		"/usr/spool/news"
/* no need to define special if you're 2.10.2; we can intuit the new active file */

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

extern int errno;

char *headers[] = {
	"From: ",
	"Reply-To: ",
	"Date: ",
	"Subject: ",
	"Newsgroups: ",
	NULL,
};

main(argc, argv)
char **argv; {
	char msgdir[256], conf[33];
	long msg;
	char *cp, *dp;
	int status;

	if (argc != 3) {
		fprintf(stderr, "Usage: uanews newsgroup[/article] conf\n");
		exit(1);
	}
#ifdef NOAUTOPATH
	strcpy(msgdir, NOAUTOPATH);
#else
	strcpy(msgdir, getpwuid(geteuid())->pw_dir);
#endif NOAUTOPATH
	strcat(msgdir, "/msgdir");
	for (cp = argv[1], dp = conf; *cp != '/' && *cp != '\0'; cp++, dp++)
		*dp = *cp;
	*dp = '\0';
	if (*cp == '\0') {
		FILE *fp;
		char tmp[512], ng[512];
		long limit, cnt, minact;
		
		if ((fp = fopen(ACTIVE, "r")) == (FILE *) 0) {
			fprintf(stderr, "Not a Usenet site (no active file)...\n", conf);
			exit(7);
		}
		while (fgets(tmp, sizeof tmp, fp) != NULL) {
			if (sscanf(tmp, "%s %ld %ld", ng, &limit, &minact) == 2)
				minact = 1;	/* < 2.10.2 */
			if (strcmp(ng, conf) == 0)
				break;
		}
		fclose(fp);
		if (strcmp(ng, conf) != 0) {
			fprintf(stderr, "I can't find a newsgroup called %s.\n", conf);
			exit(13);
		}
		if (minact == limit) {
			fprintf(stderr, "Newsgroup %s is empty.\n", conf);
			exit(14);
		}
		sprintf(tmp, "%s/%s/%s", msgdir, argv[2], NEWSRC);
		if ((fp = fopen(tmp, "r")) != (FILE *) 0) {
			char tmp2[512];

			while (fgets(tmp, sizeof tmp, fp) != (char *) 0) {
				sscanf(tmp, "%[^:]: %ld", tmp2, &cnt);
				if (strcmp(tmp2, ng) == 0)
					break;
			}
			if (strcmp(tmp2, ng) == 0)
				minact = cnt + 1;
			fclose(fp);
		}
		printf("News articles %d to %d\n", minact, limit);
		for (cnt = 0, msg = minact; msg <= limit; msg++) {
			if ((status = copynews(msgdir, conf, msg, argv[2])) > 0)
				exit(status);
			else if (status == 0)
				cnt++;
		}
		printf("%ld news articles posted from %s to %s.\n", cnt, conf, argv[2]);
		exit(cnt == 0? 9: 0);
	}
	msg = atol(++cp);
	if (Index(argv[2], '/') != (char *) 0) {
		fprintf(stderr, "Usage: uanews newsgroup[/article] conf\n");
		exit(1);
	}
	if ((status = copynews(msgdir, conf, msg, argv[2])) == -1)
		fprintf(stderr, "Couldn't read article %ld of %s\n", msg, conf);
	exit(status);
}

copynews(base, ng, art, dest)
char *base, *ng, *dest;
long art; {
	char path[512], temp[1024], ngpath[512];
	int cnt, inhdr;
	long newmsg;
	FILE *ifp, *ofp;
	struct stat sbuf;
	char *cp, *dp;
	
	for (cp = ng, dp = ngpath; *cp != '\0'; cp++, dp++)
		if (*cp == '.')
			*dp = '/';
		else
			*dp = *cp;
	sprintf(path, "%s/%s/%ld", NEWSDIR, ngpath, art);
	if (stat(path, &sbuf) < 0)
		return -1;	/* likely to be common... */
	sprintf(path, "%s/%s/himsg", base, dest);
	if ((ifp = fopen(path, "r")) == (FILE *) 0) {
		fprintf(stderr, "Conference %s: missing himsg...\n", dest);
		return 2;
	}
	fgets(temp, sizeof temp, ifp);
	fclose(ifp);
	if ((newmsg = atol(temp)) < 0) {
		fprintf(stderr, "Conference %s: invalid himsg...\n", dest);
		return 2;
	}
	newmsg++;
	sprintf(path, "%s/%s/%ld", NEWSDIR, ngpath, art);
	if ((ifp = fopen(path, "r")) == (FILE *) 0) {
		fprintf(stderr, "Are you certain that you have permission to access news?\n");
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
	inhdr = 1;
	while (fgets(temp, sizeof temp, ifp) != (char *) 0) {
		if (temp[0] == '\n')
			inhdr = 0;
		if (inhdr) {
			for (cnt = 0; headers[cnt] != NULL; cnt++)
				if (strncmp(temp, headers[cnt], strlen(headers[cnt])) == 0)
					break;
			if (headers[cnt] == NULL)
				continue;
		}
		fputs(temp, ofp);
		if (ferror(ofp)) {
			fprintf(stderr, "Write error on %s/%ld\n", dest, newmsg);
			exit(5);	/* fatal! */
		}
	}
	if (ferror(ifp)) {
		fprintf(stderr, "Read error on %s/%ld\n", ng, art);
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
	sprintf(path, "/tmp/uan%05d", getpid());
	if ((ofp = fopen(path, "w")) == (FILE *) 0) {
		fprintf(stderr, "Who moved /tmp?!\n");
		return 16;
	}
	sprintf(path, "%s/%s/%s", base, dest, NEWSRC);
	dp = "-";
	if ((ifp = fopen(path, "r")) != (FILE *) 0) {
		while (fgets(temp, sizeof temp, ifp) != (char *) 0) {
			for (cp = temp; *cp != '\n' && *cp != ':'; cp++)
				;
			if (*cp == '\n')
				continue;	/* silent cleanup */
			else
				*cp = '\0';
			if (strcmp(temp, ng) == 0) {
				sprintf(temp, "%s: %ld\n", ng, art);
				dp = "#";
			}
			else
				*cp = ':';
			fputs(temp, ofp);
		}
		fclose(ifp);
	}
	if (*dp != '#')
		fprintf(ofp, "%s: %ld\n", ng, art);
	fclose(ofp);
	sprintf(path, "/tmp/uan%05d", getpid());
	if ((ifp = fopen(path, "r")) == (FILE *) 0) {
		fprintf(stderr, "Can't reopen temp file, aborting...\n");
		return 16;
	}
	sprintf(path, "%s/%s/%s", base, dest, NEWSRC);
	if ((ofp = fopen(path, "w")) == (FILE *) 0) {
		fprintf(stderr, "Conference %s: can't update news link file (E:%d)\n", dest, errno);
		return 17;
	}
	while (fgets(temp, sizeof temp, ifp) != (char *) 0)
		fputs(temp, ofp);
	fclose(ofp);
	fclose(ifp);
	sprintf(path, "/tmp/uan%05d", getpid());
	unlink(path);	/* not a tragedy if this fails... */
	return 0;
}
E 1
