/*
 *	@(#)message.c	1.1 (TDI) 2/3/87
 *	@(#)Copyright (C) 1984, 85, 86, 87 by Brandon S. Allbery.
 *	@(#)This file is part of UNaXcess version 1.0.2.
 *
 *	Permission is hereby granted to copy and distribute this program
 *	freely.  Permission is NOT given to modify this program or distribute
 *	it at cost, except for charging a reasonable media/copying fee.
 */

#ifndef lint
static char _FileID_[] = "@(#)message.c	1.1 (TDI) 2/3/87";
static char _UAID_[]   = "@(#)UNaXcess version 1.0.2";
#endif lint

#include "ua.h"

extern struct cmd maincmd[];

selmsg(fn)
int (*fn)(); {
	char line[256], *p;
	short lomsg, himsg;
	FILE *f;

	sprintf(line, "%s/%s/himsg", MSGBASE, conference);
	if ((f = fopen(line, "r")) == NULL) {
		log("Error %d opening %s", errno, line);
		if (strcmp(conference, "general") == 0)
			panic("conf");
		writes("I can't find the high message file.  Moving back to general...");
		strcpy(conference, "general");
		return 1;
	}
	fgets(line, 32, f);
	fclose(f);
	himsg = atoi(line);
	writef("Forward, Reverse, Individual, or New (RETURN to abort): ");
	line[0] = readc();
	log("Mode: %c", line[0]);
	switch (line[0]) {
	case 'F':
		lomsg = 1;
		break;
	case 'R':
		lomsg = himsg;
		himsg = 1;
		break;
	case 'I':
		writef("Enter message number: ");
		reads(line);
		log("Message: %s", line);
		if ((lomsg = atoi(line)) < 1 || lomsg > himsg) {
			writes("No such message.");
			log("No such message.");
			return 1;
		}
		domsg(conference, lomsg, lomsg, fn);
		return 1;
	case 'N':
		lomsg = 0;
		break;
	case ' ':
		return 1;
	default:
		writes("What?  Valid commands are F, R, I, N, or ENTER.");
		log("Illegal search mode.");
		return 1;
	}
	if (lomsg != 0) {
		writef("Starting message (%d): ", lomsg);
		reads(line);
		log("Start: %s", line);
		if (line[0] != 0)
			if (atoi(line) < 1 || (lomsg > 1 && atoi(line) > lomsg)) {
				writes("Bad message number.");
				log("Bad message number.");
				return 1;
			}
			else
				lomsg = atoi(line);
		writef("Ending message (%d): ", himsg);
		reads(line);
		log("End: %s", line);
		if (line[0] != 0)
			if (atoi(line) < 1 || (himsg > 1 && atoi(line) > himsg)) {
				writes("Bad message number.");
				log("Bad message number.");
				return 1;
			}
			else
				himsg = atoi(line);
	}
	domsg(conference, lomsg, himsg, fn);
	return 1;
}

readmsg() {
	return selmsg(doread);
}

scanmsg() {
	return selmsg(doscan);
}

doread(msg, conf, mnum)
char *msg, *conf;
short mnum; {
	char ch;

	writef("\nMessage %d of %s:\n", mnum, conf);
	if (isprivate(msg)) {
		writes("This message is private.");
		return 1;
	}
	cat(msg);

DR_Loop:
	writef("\nContinue, Stop, %sEnter a message, Unsubscribe, Xecute, or Reply? C\b", (user.u_access == A_WITNESS || s_cmp(getowner(msg), user.u_name) == 0? "Kill, ": ""));
	if (!isatty(0)) {
		ch = ' ';
		writes("C");
	}
	else
		ch = readc();
	log("C/S/K/E/U/R/X: %c", ch);
	switch (ch) {
	case 'c':
	case 'C':
	case '\n':
	case '\r':
	case ' ':
		return 1;
	case 'U':
	case 'u':
		unsubscribe(conf);
		return 0;
	case 's':
	case 'S':
		return 0;
	case 'r':
	case 'R':
		reply(msg, conf);
		goto DR_Loop;
	case 'k':
	case 'K':
		if (unlink(msg) < 0) {
			writef("Can't kill message: %d", mnum);
			log("Error %d unlinking %s", errno, msg);
			goto DR_Loop;
		}
		log("Deleted %s:%d", conference, mnum);
		return 1;
	case 'E':
	case 'e':
		enter("");
		goto DR_Loop;
	case 'x':
	case 'X':
		__recurse++;
		pcmd("Command (? for help, G to return): ", maincmd, (char *) 0);
		goto DR_Loop;
	default:
		writef("What?  Please enter one of C, S, %sE, U, X, or R.", (user.u_access == A_WITNESS || s_cmp(getowner(msg), user.u_name) == 0? "K, ": ""));
		goto DR_Loop;
	}
}

msgok(file)
char *file; {
	FILE *fp;

	if ((fp = fopen(file, "r")) == NULL)
	return 0;
	fclose(fp);
	return 1;
}

doscan(msg, conf, mnum)
char *msg, *conf;
short mnum; {
	char line[1024];
	FILE *f;
	short dflag, fflag, tflag, sflag;

	if ((f = fopen(msg, "r")) == NULL) {
		writes("Cannot open file.");
		log("Error %d opening %s", errno, msg);
		return 1;
	}
	writef("\nMessage %d of %s: \n", mnum, conf);
	dflag = fflag = tflag = sflag = 0;
	if (isprivate(msg))
		writes("Message is private.");
	else {
		while (fgets(line, 1024, f) != NULL) {
			if (line[0] == '\n')
				break;
			if (!dflag && strncmp(line, "Date: ", 6) == 0) {
				writef("%s", line);
				dflag++;
				continue;
			}
			if (!fflag && strncmp(line, "From: ", 6) == 0) {
				writef("%s", line);
				fflag++;
				continue;
			}
			if (!tflag && strncmp(line, "To: ", 4) == 0) {
				writef("%s", line);
				tflag++;
				continue;
			}
			if (!sflag && strncmp(line, "Subject: ", 9) == 0) {
				writef("%s", line);
				sflag++;
				continue;
			}
			if (!sflag && strncmp(line, "Subject (Private): ", 19) == 0) {
				writef("%s", line);
				sflag++;
				continue;
			}
		}
		if (!tflag)
		writes("To: All");
	}
	fclose(f);
	writes("--------------------------------");
	return 1;
}

domsg(conf, lomsg, himsg, fn)
char *conf;
short lomsg, himsg;
int (*fn)(); {
	short mcnt;
	char tmps[256];
	struct _himsg *ptr;

	for (ptr = hicnts; ptr != NULL; ptr = ptr->hi_next)
		if (strcmp(conf, ptr->hi_conf) == 0)
			break;
	if (ptr == NULL) {
/*
 * Query: how do you silence lint's complaints about calloc() casting?
 */
		if ((ptr = (struct _himsg *) calloc((unsigned) 1, sizeof (struct _himsg))) == NULL) {
			log("Error %d allocating _himsg for %s", errno, conf);
			panic("alloc");
		}
		ptr->hi_next = hicnts;
		hicnts = ptr;
		ptr->hi_uns = HI_SUBSCR;
		strcpy(ptr->hi_conf, conf);
		ptr->hi_num = 0;
	}
	if (lomsg == 0)			/* read new messages */
		for (mcnt = ptr->hi_num + 1; mcnt <= himsg; mcnt++) {
			sprintf(tmps, "%s/%s/%d", MSGBASE, conf, mcnt);
			if (msgok(tmps) <= 0)
				continue;
			if (!(*fn)(tmps, conf, mcnt))
				break;
		}
	else if (lomsg <= himsg)		/* forward or individual read */
		for (mcnt = lomsg; mcnt <= himsg; mcnt++) {
			sprintf(tmps, "%s/%s/%d", MSGBASE, conf, mcnt);
			if (msgok(tmps) <= 0)
				continue;
			if (!(*fn)(tmps, conf, mcnt))
				break;
		}
	else
		for (mcnt = lomsg; mcnt >= himsg; mcnt--) {
			sprintf(tmps, "%s/%s/%d", MSGBASE, conf, mcnt);
			if (msgok(tmps) <= 0)
				continue;
			if (!(*fn)(tmps, conf, mcnt))
				break;
		}
	ptr->hi_num = himsg;
	writehigh(hicnts);
}

readnew() {
	DIR *dp;
	struct direct *dirp;
	FILE *hp;
	short himsg;
	char line[256], ch;

	if ((dp = opendir(MSGBASE)) == NULL) {
		log("Error %d reading dir %s/", errno, MSGBASE);
		panic("msgdir");
	}
	while ((dirp = readdir(dp)) != NULL) {
		if (dirp->d_name[0] == '.')
			continue;
		if (isunsub(dirp->d_name))
			continue;
		log("Reading %s.", dirp->d_name);
		if (user.u_access != A_WITNESS && parms.ua_xrc)
			if (!isrcmem(user.u_name, dirp->d_name)) {
				log("Skipping Restricted conference.");
				continue;
			}
		writef("\nExamining conference %s...\n", dirp->d_name);
		sprintf(line, "%s/%s/himsg", MSGBASE, dirp->d_name);
		if ((hp = fopen(line, "r")) == NULL) {
			log("Error %d opening %s", errno, line);
			writes("Can't open high message file.");
			continue;
		}
		fgets(line, 32, hp);
		fclose(hp);
		himsg = atoi(line);
		domsg(dirp->d_name, 0, himsg, doread);
	
RN_Loop:
		writef("\nNext conference, Unsubscribe, or Stop? N\b");
		if (!isatty(0)) {
			writes("N");
			ch = ' ';
		}
		else
			ch = readc();
		log("Next/Unsub/Stop: %c", ch);
		switch (ch) {
		case 'N':
		case 'n':
		case ' ':
		case '\r':
		case '\n':
			break;
		case 'U':
		case 'u':
			unsubscribe(dirp->d_name);
			break;
		case 'S':
		case 's':
			closedir(dp);
			return 1;
		default:
			writes("Please enter one of N, U, or S.");
			goto RN_Loop;
		}
	}
	closedir(dp);
	return 1;
}

enter() {
	char to[256], subj[256], *p, line[256];
	short pflag;

	if (user.u_access == A_GUEST && strcmp(conference, "guest") != 0) {
		log("Security violation:  GUEST entering messages.");
		writes("You aren't allowed to enter messages in this conference.");
		return 1;
	}
	writef("Who is this message to (ENTER to abort)? ");
	reads(line);
	log("To: %s", line);
	if (line[0] == '\0')
		return 1;
	for (p = line; *p != '\0'; p++)
		*p = ToLower(*p);
	strcpy(to, line);
	writef("Subject: ");
	reads(line);
	if (line[0] == '\0')
		return 1;
	strcpy(subj, line);
	log("Subject: %s", line);
	mkmsg(to, subj, conference, 0);
	return 1;
}

reply(msg, conf)
char *msg, *conf; {
	char to[256], subj[256], line[1024];
	short fflag, sflag, pflag;
	FILE *f;

	if (user.u_access == A_GUEST && strcmp(conf, "guest") != 0) {
		log("Security violation:  GUEST entering messages");
		writes("You aren't allowed to enter messages.");
		return;
	}
	if ((f = fopen(msg, "r")) == NULL) {
		log("Error %d opening %s", errno, msg);
		writes("Can't re-open message file.");
		return;
	}
	fflag = sflag = 0;
	strcpy(to, "All\n");
	strcpy(subj, "Re: Orphaned Response\n");	/* now you know... */
	while (fgets(line, 1024, f) != NULL) {
		if (line[0] == '\n')
			break;
		if (!fflag && strncmp(line, "From: ", 6) == 0) {
			strcpy(to, &line[6]);
			fflag++;
			continue;
		}
		if (!sflag && strncmp(line, "Subject: ", 9) == 0) {
			if (strncmp(&line[9], "Re: ", 4) == 0)
				strcpy(subj, &line[9]);
			else
				strcpy(&subj[4], &line[9]);
			sflag++;
			pflag = 0;
			continue;
		}
		if (!sflag && strncmp(line, "Subject (Private): ", 19) == 0) {
			if (strncmp(&line[19], "Re: ", 4) == 0)
				strcpy(subj, &line[19]);
			else
				strcpy(&subj[4], &line[19]);
			sflag++;
			pflag = 1;
			continue;
		}
	}
	fclose(f);
	to[strlen(to) - 1] = '\0';			/* get rid of trailing nl */
	subj[strlen(subj) - 1] = '\0';
	mkmsg(to, subj, conf, pflag);
}

doqscan(msg, conf, mnum)
char *msg, *conf;
short mnum; {
	char line[1024];
	FILE *f;

#ifdef lint
	puts(conf);	/* shut lint up about "arg not used" */
#endif lint
	if ((f = fopen(msg, "r")) == NULL) {
		writes("Cannot open file.");
		log("Error %d opening %s", errno, msg);
		return 1;
	}
	writef("%5d. ", mnum);
	if (isprivate(msg))
		writes("Private message.");
	else
		while (fgets(line, 1024, f) != NULL) {
			if (line[0] == '\n')
				break;
			if (strncmp(line, "Subject: ", 9) == 0) {
				writef("%s", &line[9]);
				break;
			}
			if (strncmp(line, "Subject (Private): ", 19) == 0) {
				writef("%s", &line[8]);		/* include privacy tag */
				break;
			}
		}
	fclose(f);
	return 1;
}

qscan() {
	return selmsg(doqscan);
}

mkmsg(to, subj, conf, dpflag)
char *to, *subj, *conf; {
	static char lockfile[] = "msgbase.lock";
	char *tempfile = mktemp("/tmp/UAmXXXXXX");
	FILE *mfp, *sfp;
	char line[1024], *p, ch, rconf[256];
	short mcnt;
	struct user ubuf;

	for (;;) {
		writef("To which conference do you wish this message to go (%s)? ", conf);
		reads(rconf);
		if (rconf[0] == '\0')
			strcpy(rconf, conf);
		if (!isconf(rconf)) {
			writes("That conference doesn't exist.");
			continue;
		}
		if (!isrcmem(user.u_name, rconf)) {
			writes("You aren't a member of that conference.");
			continue;
		}
		if (user.u_access != A_WITNESS && parms.ua_roc && conf[0] == 'r' && conf[1] == '-') {
			writes("That conference is read-only.  Try dropping the R- prefix.");
			continue;
		}
		break;
	}
	if (parms.ua_pm) {
		writef("Is this message to be private? %c\b", (dpflag? 'Y': 'N'));
		line[0] = readc();
		log("Private? %c", line[0]);
		if (line[0] == 'Y')
			dpflag = 1;
		else if (line[0] == 'N')
			dpflag = 0;
	}
	if (dpflag && !getuser(to, &ubuf)) {
		writef("You can't send a private message to %s; he's unregistered.\n", upstr(to));
		log("Attempted private message to unregistered user.");
		return 0;
	}
	if ((mfp = fopen(tempfile, "w")) == NULL) {
		log("Error %d opening %s", errno, tempfile);
		panic("tmp");
	}
	for (p = to; *p != '\0'; p++)
		*p = ToUpper(*p);
	fprintf(mfp, "To: %s\nSubject%s: %s\n\n", to, (dpflag? " (Private)": ""), subj);
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
			writes("Message entry commands:\n\nL - List message\nC - Continue message entry\nE - Edit message\nS - Save message\nA - Abort message\n");
			break;
		case 'S':
			writes("Saving message...");
			mklock(lockfile);
			sprintf(line, "%s/%s/himsg", MSGBASE, rconf);
			if ((sfp = fopen(line, "r")) == NULL) {
				log("Error %d opening %s", errno, line);
				rmlock(lockfile);
				unlink(tempfile);
				panic("himsg");
			}
			fgets(line, 32, sfp);
			fclose(sfp);
			mcnt = atoi(line) + 1;
			sprintf(line, "%s/%s/%d", MSGBASE, rconf, mcnt);
			if ((sfp = fopen(line, "w")) == NULL) {
				log("Error %d opening %s", errno, line);
				unlink(tempfile);
				rmlock(lockfile);
				panic("msg");
			}
			fprintf(sfp, "Date: %s\nFrom: %s\n", longdate(), upstr(user.u_name));
			if ((mfp = fopen(tempfile, "r")) == NULL) {
				fclose(sfp);
				log("Error %d opening %s", errno, tempfile);
				unlink(tempfile);
				unlink(line);
				rmlock(lockfile);
				panic("tmp");
			}
			while (fgets(line, 1024, mfp) != NULL)
				fputs(line, sfp);
			fclose(sfp);
			fclose(mfp);
			unlink(tempfile);
			sprintf(line, "%s/%s/himsg", MSGBASE, rconf);
			if ((sfp = fopen(line, "w")) == NULL) {
				log("Error %d opening %s", errno, line);
				panic("himsg_w");
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

edit(file)
char *file; {
	char find[256], replace[256], buf[1024], tempfile[80];
	char *bufp, *strp;
	long offset;
	FILE *src, *tmp;
	short ch, fch;
	
	sprintf(tempfile, "/tmp/UaEd%05d", getpid());
	writef("At the \"Find:\" prompt, enter the string to locate; pressing ENTER / RETURN alone exits the editor.  At each occurrence of the string you will be ");
	writef("shown that part of the message and prompted to Replace, Skip to the next occurence, or Quit the search.  If you Replace, you will be asked ");
	writef("for the replacement string; pressing ENTER or RETURN alone indicates that you wish to ");
	writef("to delete the string.\n\n");
	for (;;) {
		writef("Find: ");
		reads(find);
		if (find[0] == '\0')
			return;
		if ((tmp = fopen(tempfile, "w")) == (FILE *) 0) {
			log("Error %d opening %s", tempfile);
			panic("msged_tmpf");
		}
		if ((src = fopen(file, "r")) == (FILE *) 0) {
			log("Error %d opening %s", file);
			panic("msged_srcf");
		}
		offset = 0L;
		strp = find;
		writes("---------------");
		for (;;) {
			while ((ch = getc(src)) != EOF) {
				offset++;
				writec(ch);
				fch = ch;
				if (ToUpper(ch) == ToUpper(*strp))
					break;
				putc(ch, tmp);
			}
			if (ch == EOF)
				break;
			strp++;
			while (*strp != '\0' && (ch = getc(src)) != EOF) {
				if (ToUpper(ch) != ToUpper(*strp))
					break;
				strp++;
			}
			if (ch == EOF && *strp != '\0') {
				for (bufp = find; bufp != strp; bufp++) {
					putc(*bufp, tmp);
					writec(*bufp);
				}
				break;
			}
			if (*strp != '\0') {
				fseek(src, offset, 0L);
				strp = find;
				putc(fch, tmp);
				continue;
			}
			writef("\b \b[%s]\n---------------\n", find);

askrepl:
			writef("Replace, Skip this one, Quit?  S\b");
			ch = readc();
			switch (ch) {
			case ' ':
			case 'S':
				fseek(src, offset, 0L);
				strp = find;
				writes("---------------");
				continue;
			case 'R':
				writef("Replacement: ");
				reads(replace);
				fputs(replace, tmp);
				offset += strlen(find);
				writes("---------------");
				continue;
			case 'Q':
				while ((ch = getc(src)) != EOF)
					putc(ch, tmp);
				break;
			default:
				putchar('\7');
				goto askrepl;
			}
			break;
		}
		writes("---------------");
		fclose(src);
		fclose(tmp);
		if (unlink(file) < 0) {
			log("Error %d unlinking %s", errno, file);
			panic("msged_unlk");
		}
		if (copylink(tempfile, file) < 0) {
			log("Error %d copylinking %s to %s", errno, tempfile, file);
			panic("msged_cplk");
		}
	}
}

input(file)
char *file; {
	FILE *fp;
	char line[256];
	char *cp;
	char ch;
	int lastwasnl;

	if ((fp = fopen(file, "a")) == NULL) {
		log("Error %d opening %s", errno, file);
		unlink(file);
		panic("tmp");
	}
	writes("\nEnter your text now.  End it by typing any of ESCAPE, CONTROL-Z or CONTROL-D on a line by itself.  Your text will be word-wrapped automatically.\n");
	log("Entering text...");
	cp = line;
	interact();
	while (((ch = getchar() & 0x7f) != '\033' && ch != '\032' && ch != '\004') || !lastwasnl) {
		if (ch == '\b' || ch == '\177')
			if (cp == line) {
				putchar('\7');
				fflush(stdout);
			}
			else {
				writec('\b');
				writec(' ');
				writec('\b');
				cp--;
			}
		else if (ch == '\n' || ch == '\r') {
			*cp++ = '\n';
			*cp++ = '\0';
			fputs(line, fp);
			cp = line;
			writec('\n');
			lastwasnl = 1;
		}
		else if (ch < ' ') {
			putchar('\7');
			fflush(stdout);
		}
		else {
			lastwasnl = 0;
			*cp++ = ch;
			writec(ch == '\t'? ' ': ch);
			if (wrapped() != 0) {
				cp -= wrapped() + 1;
				ch = *cp;
				*cp = '\0';
				fputs(line, fp);
				cp = line + strlen(line);
				*cp = ch;
				strcpy(line, cp);
				cp = line + wrapped() + 1;
			}
		}
	}
	buffer();
	fclose(fp);
}
