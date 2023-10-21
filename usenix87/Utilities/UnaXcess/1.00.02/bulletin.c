/*
 *	bulletin.c	1.2 (TDI) 2/3/87
 *	Copyright (C) 1984, 85, 86, 87 by Brandon S. Allbery.
 *	This file is part of UNaXcess version 1.0.2.
 */

#ifndef lint
static char _FileID_[] = "@(#)bulletin.c	1.2 (TDI) 2/3/87";
static char _UAID_[]   = "@(#)UNaXcess version 1.0.2";
#endif lint

#include "ua.h"

bulletin() {
	short mcnt, himotd;
	char tmps[256];
	FILE *fp;

	if (user.u_access == A_MKUSER)
		return 1;
	sprintf(tmps, "%s/himotd", MOTD);
	if ((fp = fopen(tmps, "r")) == NULL) {
		log("Error %d opening %s", errno, tmps);
		panic("himotd");
	}
	fgets(tmps, 32, fp);
	fclose(fp);
	himotd = atoi(tmps);
	if (s_cmp(user.u_name, "guest") == 0)
		mcnt = 1;
	else
		mcnt = user.u_nbull + 1;
	for (; mcnt <= himotd; mcnt++) {
		sprintf(tmps, "%s/%d", MOTD, mcnt);
		if (!readmotd(tmps, mcnt))
			break;
	}
	return 1;
}

readmotd(motd, mnum)
char *motd;
short mnum; {
	char ch;

	writef("Bulletin #%d:\n", mnum);
	cat(motd);
	writef("\nContinue or Stop? C\b");
	ch = readc();
	log("C/S? %c", ch);
	return ToLower(ch) != 's';
}

mkbull() {
	static char lockfile[] = "bulletin.lock";
	char *tempfile = mktemp("/tmp/UAbXXXXXX");
	FILE *mfp, *sfp;
	char line[1024], *p, ch;
	short mcnt;

	if (user.u_access != A_WITNESS) {
		log("Attempted mkbull by non-FW.");
		writes("You aren't permitted to enter bulletins.");
		return 1;
	}
	if ((mfp = fopen(tempfile, "w")) == NULL) {
		log("Error %d opening %s", errno, tempfile);
		panic("tmp");
	}
	fclose(mfp);
	input(tempfile);
	for (;;) {
		writef("\nList, Continue entry, Edit, Save, or Abort? ");
		ch = readc();
		log("Edit command: %c", ch);
		switch (ch) {
		case 'L':
			writes("\n--------------------");
			cat(tempfile);
			writes("--------------------");
			break;
		case 'C':
			input(tempfile);
			break;
		case 'E':
			edit(tempfile);
			break;
		case 'A':
			writef("Do you really want to abort this edit? N\b");
			line[0] = readc();
			log("Abort? %c", line[0]);
			if (line[0] == 'Y') {
				unlink(tempfile);
				return 0;
			}
			break;
		case '?':
			writes("Bulletin create commands:\n\nL - List message\nC - Continue message entry\nE - Edit message\nS - Save message\nA - Abort message\n");
			break;
		case 'S':
			writes("Saving bulletin...");
			mklock(lockfile);
			sprintf(line, "%s/himotd", MOTD);
			if ((sfp = fopen(line, "r")) == NULL) {
				log("Error %d opening %s", errno, line);
				rmlock(lockfile);
				unlink(tempfile);
				panic("himotd");
			}
			fgets(line, 32, sfp);
			fclose(sfp);
			mcnt = atoi(line) + 1;
			sprintf(line, "%s/%d", MOTD, mcnt);
			if ((sfp = fopen(line, "w")) == NULL) {
				log("Error %d opening %s", errno, line);
				unlink(tempfile);
				rmlock(lockfile);
				panic("motd");
			}
			fprintf(mfp, "UNaXcess Conferencing, Version 1.00.02\nDate: %s\nFrom: %s\n\n", longdate(), upstr(user.u_name));
			if ((mfp = fopen(tempfile, "r")) == NULL) {
				fclose(sfp);
				log("Error %d opening %s", errno, tempfile);
				unlink(tempfile);
				unlink(line);
				rmlock(lockfile);
				panic("btmp");
			}
			while (fgets(line, 1024, mfp) != NULL)
				fputs(line, sfp);
			fclose(sfp);
			fclose(mfp);
			unlink(tempfile);
			sprintf(line, "%s/himotd", MOTD);
			if ((sfp = fopen(line, "w")) == NULL) {
				log("Error %d opening %s", errno, line);
				panic("himotd_w");
			}
			fprintf(sfp, "%d\n", mcnt);
			fclose(sfp);
			rmlock(lockfile);
			return 1;
		default:
			writes("Please enter L, C, E, S, or A; or ? for help.");
		}
	}
}
