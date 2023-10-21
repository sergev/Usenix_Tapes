
/* RCS Info: $Revision: 1.5 $ on $Date: 86/04/02 10:33:36 $
 *           $Source: /ic4/faustus/src/spellfix/RCS/spellfix.c,v $
 * Copyright (c) 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
 *	Permission is granted to do anything with this code except sell it
 *	or remove this message.
 *
 * This is the main control program for the spellfix system. Usage is
 *	spellfix document
 */

#include "spellfix.h"
#include <sys/file.h>
#include <curses.h>
#include <pwd.h>

int debug = 1;

main(ac, av, ep)
	char **av, **ep;
{
	char buf[BUFSIZ];
	char *tempf1 = "/tmp/spf1XXXXXX";
	char *tempf2 = "/tmp/spf2XXXXXX";
	char *tempf3 = "/tmp/spf3XXXXXX";
	char **words[NDICTS];
	int nwords[NDICTS], i, c;
	FILE *fp, *out, *fopen();
	char *mktemp(), *fixword(), *getbuf(), *badword, *goodword, *s;
	struct passwd *pwd, *getpwuid();

	if (!strcmp(av[0], "spell")) {
		/* Spell emulation mode. */
	} if (ac != 2) {
		fprintf(stderr, "Usage: %s document\n", av[0]);
		exit(1);
	}

	/* This is cleaner than letting sh find the error. */
	if (access(av[1], 0)) {
		fprintf(stderr, "Error: can't open %s.\n", av[1]);
		exit(1);
	} else if (access(av[1], W_OK | R_OK)) {
		fprintf(stderr, "Error: %s must be readable and writable.\n",
				av[1]);
			exit(1);
	}

	printf("Starting up... "); fflush(stdout);
	tempf1 = mktemp(tempf1);
	tempf2 = mktemp(tempf2);
	tempf3 = mktemp(tempf3);

	/* Read the .spellrc file... */
	pwd = getpwuid(getuid());
	if (!pwd->pw_dir) {
		fprintf(stderr, "Hey, you don't have a home directory.\n");
		exit(1);
	}
	sprintf(buf, "%s/.spellrc", pwd->pw_dir);
	if (!access(buf, R_OK))
		nwords[0] = readdict(buf, &words[0]);
	else
		nwords[0] = 0;

	/* First prep the doc. We won't use prep because deroff is
	 * better and more standard.
	 */
	sprintf(buf, "/usr/bin/deroff -w < %s | /usr/bin/sort -u > %s", 
			av[1], tempf1);
	if (system(buf))
		exit(1);
	
	/* Now make up the decomposition list of the words. */
	sprintf(buf, "%s/baseword < %s > %s", BINDIR, tempf1, tempf2);
	if (system(buf))
		exit(1);
	
	/* Seperate them into bad (tempf1) and good (tempf3) files. */
	printf("done.\nIdentifying misspelled words... "); fflush(stdout);
	sprintf(buf, "%s/spellcheck %s %s < %s", BINDIR, tempf1, tempf3, 
			tempf2);
	if (system(buf))
		exit(1);

	unlink(tempf2);

	i = 0;
	fp = fopen(tempf1, "r");
	while ((c = getc(fp)) != EOF)
		if (c == '\n')
			i++;
	fclose(fp);

	/* Now read in the good file as a dictionary. */
	printf("done (%d misspelled word%s).\nReading dictionaries...\n", i,
			(i == 1) ? "" : "s");
	nwords[1] = readdict(tempf3, &words[1]);
	printf("%d word%s in document correct...\n", nwords[1],
			(nwords[1] == 1) ? "" : "s");
	if (nwords[1] <= 0) {
		/* Probably this should be an error. */
		printf("Wow, not a single word spelled right!\n");
	}

	unlink(tempf3);

	sprintf(buf, "%s/freqwords", BINDIR);
	nwords[2] = readdict(buf, &words[2]);
	printf("Read %d frequently used words...\n", nwords[2]);
	if (nwords[2] <= 0)
		exit(1);
	
	nwords[3] = readdict((char *) NULL, &words[3]);
	printf("And %d words from the dictionary.\n", nwords[3]);
	if (nwords[3] <= 0)
		exit(1);
	
	/* Now for each word, try to correct it. */
	if (!(fp = fopen(tempf1, "r"))) {
		perror(tempf1);
		exit(1);
	}
	if (!(out = fopen(tempf2, "w"))) {
		perror(tempf2);
		exit(1);
	}

	initscr();
	crmode();

	/* It looks like curses likes to trash my environment... */
	for (i = 0; ep[i]; i++)
		if (!index(ep[i], '='))
			ep[i] = "CURSES=shit";

	/* Read words from tempf1, get the corrections, and write
	 * bad/good pairs to tempf2.
	 */
	i = 0;
	while (fgets(buf, BUFSIZ, fp)) {
		goodword = fixword(buf, words, nwords, 4, av[1]);
		if (!goodword)
			break;
		else if (goodword == (char *) 1) {
			system("stty -tabs");
			unlink(tempf1);
			unlink(tempf2);
			exit(0);
		}
		badword = buf;
		for (s = buf; isvalid(*s); s++)
			;
		*s = '\0';
		if (goodword && *goodword && strcmp(badword, goodword)) {
			fprintf(out, "%s %s\n", badword, goodword);
			i++;
		}
	}
	if (i)
		sprintf(buf, "%d words corrected, ok to write out changes? ",
				i);
	else
		strcpy(buf, "No words corrected.");
	mvaddstr(22, 0, buf);
	refresh();
	if (i)
		strcpy(buf, getbuf());
	endwin();
	system("stty -tabs");	/* Damn... */
	fclose(out);
	fclose(fp);
	unlink(tempf1);

	if (!i) {
		unlink(tempf2);
		putchar('\n');
		exit(0);
	}
	if ((buf[0] != 'y') && (buf[0] != 'Y')) {
		printf("\nOk, aborting... Changes saved in %s.changes.\n",
				av[1]);
		printf("This file is valid input for wordchange(1).\n");
		sprintf(buf, "/bin/cp %s %s.changes", tempf2, av[1]);
		system(buf);
		unlink(tempf2);
		exit(0);
	}

	printf(" ... "); fflush(stdout);
	sprintf(buf, "%s/wordchange %s < %s", BINDIR, av[1], tempf2);
	if (system(buf))
		exit(1);
	unlink(tempf2);
	printf("done.\nSo long...\n");
	exit(0);
}

