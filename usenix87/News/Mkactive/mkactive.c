/*
 * mkactive
 *
 * Rebuilds a current version of the active file, given an old version
 * with the right groups but the wrong numbers.  Input is sactive,
 * output is nactive, both in current directory.
 * Also produces nhistory, for use by expire.
 *
 * From: Jim Rees (Apollo Computer) <rees@apollo.uucp>
 *
 * This program serves two useful purposes.  It rebuilds the active file,
 * and it produces a history file with entries for suspicious looking articles.
 * This can be used to clean out old news articles which for some reason
 * have been dropped from the history file.
 *
 * Typical usage might be something like:
 *
 * cp active sactive		# I have cron do this once a night
 * mkactive -h
 * mv nactive active
 * cat nhistory >>history
 * expire
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>

char SPOOL[] =	"/news/spool";
char sactive[] = "sactive";
char nactive[] = "nactive";
char nhist[] = "nhistory";

int hflag;

main(ac, av)
int ac;
char *av[];
{
	FILE *sfp, *nfp, *hfp = NULL;
	char buf[64], group[64];
	char *p, c;
	int min, max, omin, omax;
	int nfields;

	while (ac > 1 && av[1][0] == '-') {
		switch (av[1][1]) {
		case 'h':
			hflag = 1;
			break;
		}
		ac--;
		av++;
	}

	sfp = fopen(sactive, "r");
	if (sfp == NULL) {
		perror(sactive);
		exit(1);
	}
	nfp = fopen(nactive, "w");
	if (nfp == NULL) {
		perror(nactive);
		exit(1);
	}
	if (hflag) {
		hfp = fopen(nhist, "w");
		if (hfp == NULL) {
			perror(nhist);
			exit(1);
		}
	}
	while (fgets(buf, sizeof buf, sfp) != NULL) {
		nfields = sscanf(buf, "%s %d %d %c", group, &omax, &omin, &c);
		if (nfields < 1)
			continue;
		if (nfields < 2)
			omax = 0;
		if (nfields < 3)
			omin = omax;
		if (nfields < 4)
			c = strncmp(group, "fa.", 3) ? 'y' : 'n';
		dogroup(group, omax, &min, &max, hfp);
		fprintf(nfp, "%s %05.5d %05.5d %c\n", group, max, min, c);
	}
	exit(0);
}

dogroup(group, omax, minp, maxp, hfp)
char *group;
int omax, *minp, *maxp;
FILE *hfp;
{
	static unsigned short list[10000];
	char dirname[64];
	int i, nart, min, max, ninety;
	int compare();

	GrToDir(group, dirname);
	nart = pass1(dirname, list);
	qsort(list, nart, sizeof (unsigned short), compare);
	/* Find lowest numbered real article */
	for (i = 0; i < nart; i++)
		if (ForReal(dirname, (int) list[i]))
			break;
	if (i >= nart)
		/* No legit articles; use old count */
		min = omax + 1;
	else
		min = list[i];
	/* Find highest numbered real article */
	for (i = nart - 1; i >= 0; i--)
		if (ForReal(dirname, (int) list[i]))
			break;
	if (i < 0)
		/* No legit articles; use old count */
		max = omax;
	else
		max = list[i];
	if (max - min > 1500)
		printf("WARNING: too many articles\n");
	ninety = (nart * 10) / 9;
	if (max - min > ninety) {
		printf("%-20s %4d articles, range %4d (%d-%d)\n",
		    group, nart, max-min, min, max);
		if (hfp != NULL)
			for (i = 0; i < nart && max - list[i] > ninety; i++)
				fprintf(hfp, "\t\t%s/%d\n", group, list[i]);
	}
	if (max < omax)
		max = omax;
	if (min > max + 1)
		min = max;
	*minp = min;
	*maxp = max;
}

pass1(dirname, list)
char *dirname;
unsigned short list[];
{
	int i, n;
	DIR *dp;
	struct direct *d;
	char buf[16];

	dp = opendir(dirname);
	if (dp == NULL) {
		perror(dirname);
		return 0;
	}
	i = 0;
	while ((d = readdir(dp)) != NULL) {
		n = atoi(d->d_name);
		if (n <= 0)
			continue;
		sprintf(buf, "%d", n);
		if (strcmp(buf, d->d_name))
			printf("strange article %s/%s\n", dirname, d->d_name);
		else
			list[i++] = n;
	}
	closedir(dp);
	return i;
}

compare(p1, p2)
unsigned short *p1, *p2;
{
	return((int) *p1 - (int) *p2);
}

GrToDir(group, dirname)
char *group, *dirname;
{
	char *p, *q;

	for (p = SPOOL, q = dirname; *p; )
		*q++ = *p++;
	*q++ = '/';
	for (p = group; *p; p++, q++)
		if (*p == '.')
			*q = '/';
		else
			*q = *p;
	*q = '\0';
}

ForReal(dirname, n)
char *dirname;
int n;
{
	char file[100];
	struct stat st;

	sprintf(file, "%s/%d", dirname, n);
	if (stat(file, &st) < 0) {
		if (unlink(file) >= 0)
			printf("removed bad file %s\n", file);
		else
			printf("can't figure out %s\n", file);
		return 0;
	}
	if (st.st_mode & S_IFREG)
		return 1;
	printf("not a file: %s\n", file);
	return 0;
}
