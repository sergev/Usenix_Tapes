/*
 *	conference.c	1.2 (TDI) 2/3/87
 *	Copyright (C) 1984, 85, 86, 87 by Brandon S. Allbery.
 *	This file is part of UNaXcess version 1.0.2.
 *
 *	Permission is hereby granted to copy and distribute this program
 *	freely.  Permission is NOT given to modify this program or distribute
 *	it at cost, except for charging a reasonable media/copying fee.
 */

#ifndef lint
static char _FileID_[] = "@(#)conference.c	1.2 (TDI) 2/3/87";
static char _UAID_[]   = "@(#)UNaXcess version 1.0.2";
#endif lint

#include "ua.h"

char conference[33];

confidx() {
	FILE *ifd;
	short himsg;
	char line[256];
	DIR *dp;
	struct direct *dfd;

	writes("\nConference                      Subscribed?  Messages  # Read  Restricted?\n");
	if ((dp = opendir(MSGBASE)) == NULL) {
		log("Error %d opening dir %s/", errno, MSGBASE);
		panic("msgdir");
	}
	while ((dfd = readdir(dp)) != NULL) {
		if (dfd->d_name[0] == '.')
			continue;
		sprintf(line, "%s/%s/himsg", MSGBASE, dfd->d_name);
		if ((ifd = fopen(line, "r")) == NULL) {
			log("No himsg in conference %s", dfd->d_name);
			continue;
		}
		fgets(line, 32, ifd);
		himsg = atoi(line);
		writef("%-32.32s     %c        %5d    %5d        %c\n", dfd->d_name, (isunsub(dfd->d_name)? 'N': 'Y'), himsg, cnread(dfd->d_name), (isrconf(dfd->d_name)? 'Y': 'N'));
		fclose(ifd);
	}
	closedir(dp);
	return 1;
}

join() {
	char line[256], *p;

	do {
		writef("Enter conference: ");
		reads(line);
		log("Enter conference: %s", line);
		if (line[0] == '\0')
			return 1;
	} while (!verify(line));
	strcpy(conference, line);
	log("Current conference is %s", conference);
	return 1;
}

verify(conf)
char *conf; {
	char *cp, line[256];

	for (cp = conf; *cp != 0; cp++) {
		if (!isprint(*cp))
			return 0;
		else if (*cp == '/' || *cp == '!' || *cp == ':')
			*cp = '.';
		else
			*cp = ToLower(*cp);
	}
	if (cp - conf > CONFSIZE)
		conf[CONFSIZE] = '\0';
	sprintf(line, "%s/%s", MSGBASE, conf);
	if (chdir(line) == -1) {
		if (errno != ENOENT) {
			log("Error %d accessing dir %s/", errno, line);
			return 0;
		}
		else
			return newconf(conf);
	}
	if (chdir(parms.ua_home) == -1) {
		log("Can't chdir to HOME (errno=%d)", errno);
		system("pwd");
		panic("isconf_cd");
	}
	if (isunsub(conf)) {
		writef("You are unsubscribed from this conference.  Rejoin? N\b");
		line[0] = readc();
		log("Unsubscribed.  Resubscribe? %c", line[0]);
		if (line[0] == 'Y')
			resubscribe(conf);
		else
			return 0;
	}
	if (user.u_access != A_WITNESS && parms.ua_xrc && !isrcmem(user.u_name, conf)) {
		log("No access to restricted conference %s", conf);
		writes("I'm sorry, but that conference has restricted membership.");
		return 0;
	}
	return 1;
}

killmsg() {
	short mnum;
	char line[256], *p;

	if (user.u_access == A_GUEST) {
		writes("You aren't authorized for this function.");
		log("Security violation:  KILL by a GUEST");
		return 1;
	}
	writef("Enter message number to kill: ");
	reads(line);
	if (line[0] == '\0')
		return 1;
	if ((mnum = atoi(line)) < 1) {
		writes("Bad message number.");
		log("Bad message number: %s", line);
		return 1;
	}
	dokill(mnum);
	return 1;
}

dokill(msg)
short msg; {
	char mfile[256];

	sprintf(mfile, "%s/%s/%d", MSGBASE, conference, msg);
	if (user.u_access != A_WITNESS && s_cmp(getowner(mfile), user.u_name) != 0) {
		writes("Sorry, you don't own that message.");
		log("Security violation:  KILL by non-owner");
		return;
	}
	if (unlink(mfile) < 0) {
		writef("No such message: %d", msg);
		log("Error %d unlinking %s", errno, mfile);
		return;
	}
	log("Deleted %s:%d", conference, msg);
}

char *getowner(file)
char *file; {
	FILE *f;
	char line[1024], *p;
	static char owner[256];

	strcpy(owner, parms.ua_sysop);
	if ((f = fopen(file, "r")) == NULL)
		return owner;
	while (fgets(line, 1024, f) != NULL)
		if (line[0] == '\n')
			break;
		else if (strncmp(line, "From: ", 6) == 0) {
			strcpy(owner, &line[6]);
			break;
		}
	fclose(f);
	for (p = owner; *p != '\0'; p++)
		*p = ToLower(*p);
	return owner;
}

newconf(conf)
char *conf; {
	char line[256];
	FILE *f;

	if (user.u_access == A_GUEST) {
		log("Security violation:  attempted MKCONF by guest");
		writes("Sorry, there is no such conference.");
		return 0;
	}
	writef("There is no conference by that name.  Do you want to create it? N\b");
	line[0] = readc();
	log("Nonexistent.  Create? %c", line[0]);
	if (line[0] != 'Y')
		return 0;
	if (parms.ua_roc && conf[0] == 'r' && conf[1] == '-')
		if (user.u_access != A_WITNESS) {
			writes("Only Fairwitnesses can make READ-ONLY conferences.  If you really want one, you will have to ask the Sysop to make you a Fairwitness.  Otherwise, try using a conference name not beginning with \"R-\".");
			log("Attempted mk of RO conf by non-FW");
			return 0;
		}
		else {
			writef("This conference will be READ-ONLY, except to Fairwitnesses.  If you want anyone to be able to add to it, answer NO and use a name not beginning with \"R-\".  Do you want to make this READ-ONLY conference? N\b");
			line[0] = readc();
			log("Read-only.  Create? %c", line[0]);
			if (line[0] != 'Y')
				return 0;
		}
#ifdef BSD
	sprintf(line, "%s/%s", MSGBASE, conf);
	if (mkdir(line, 0600) < 0) {
		log("Mkconf of %s failed", conf);
		writes("Hmmm... guess you aren't allowed.");
		return 0;
	}
	chown(line, geteuid(), getegid());
#else  !BSD
	sprintf(line, "exec mkconf %s/%s %d", MSGBASE, conf, geteuid());
	if (system(line) != 0) {
		log("Mkconf of %s failed.", conf);
		writes("Hmmm... guess you aren't allowed.");
		return 0;
	}
#endif BSD
	log("New conference: %s", conf);
	sprintf(line, "%s/%s/himsg", MSGBASE, conf);
	if ((f = fopen(line, "w")) == NULL) {
		log("Error %d opening %s", line);
		writes("Can't create high message file.  Strange...");
		return 0;
	}
	fputs("0\n", f);
	fclose(f);
	writes("You will now be placed in the message editor to make a message describing this conference.  It will be addressed to, and readable by, all users.");
	mkmsg("All", "This conference", conf, 0);
	return 1;
}

isprivate(msg)
char *msg; {
	FILE *fp;
	char line[1024], to[1024], from[1024];
	short pflag;
	register char *cp;

	if (user.u_access == A_WITNESS)
		return 0;
	if ((fp = fopen(msg, "r")) == NULL)
		return 0;
	strcpy(to, "All");
	pflag = 0;
	while (fgets(line, 1024, fp) != NULL) {
		if (line[0] == '\n')
			break;
		else if (strncmp(line, "To: ", 4) == 0)
			strcpy(to, &line[4]);
		else if (strncmp(line, "From: ", 6) == 0)
			strcpy(from, &line[6]);
		else if (strncmp(line, "Subject (Private): ", 19) == 0)
			pflag = 1;
	}
	fclose(fp);
	for (cp = to; *cp != '\0'; cp++)	/* OOPS!  2/3/87 ++bsa */
		if (*cp == '\n') {
			*cp = '\0';
			break;
		}
	for (cp = from; *cp != '\0'; cp++)
		if (*cp == '\n') {
			*cp = '\0';
			break;
		}
	if (!pflag)
		return 0;
	if (s_cmp(user.u_name, to) == 0)
		return 0;
	else if (s_cmp(user.u_name, from) == 0)
		return 0;
	else {
		log("Message %s is private.", msg);
		return 1;
	}						/* end mods 2/3/87 */
}

isunsub(conf)
char *conf; {
	struct _himsg *hip;

	for (hip = hicnts; hip != NULL; hip = hip->hi_next)
		if (strcmp(hip->hi_conf, conf) == 0)
			break;
	return (hip != NULL && hip->hi_uns == HI_UNSUB);
}

unsubscribe(conf)
char *conf; {
	struct _himsg *hip, *workp;
	char line[512];
	
	if (s_cmp(conf, "general") == 0) {
		writes("Can't unsubscribe the general conference.");
		log("Attempted to unsubscribe to general.");
		return;
	}
	if (s_cmp(conf, user.u_lconf) == 0) {
		writef("Unsubscribe to login conference? N\b");
		line[0] = readc();
		log("Unsub login conf? %c", line[0]);
		if (line[0] != 'Y')
			return;
		strcpy(user.u_lconf, "general");
	}
	for (hip = hicnts; hip != NULL; hip = hip->hi_next)
		if (strcmp(hip->hi_conf, conf) == 0)
			break;
	if (hip != NULL)
		hip->hi_uns = HI_UNSUB;
	else {
		if ((workp = (struct _himsg *) calloc((unsigned) 1, sizeof (struct _himsg))) == NULL) {
			log("Error %d allocating _himsg for %s", errno, conf);
			panic("alloc");
		}
		strcpy(workp->hi_conf, conf);
		workp->hi_num = 0;
		workp->hi_next = hicnts;
		hicnts = workp;
		workp->hi_uns = HI_UNSUB;
	}
	writehigh(hicnts);
	log("Unsubscribed to %s", conf);
	writef("Unsubscribed to conference %s.\n", conf);
}

resubscribe(conf)
char *conf; {
	struct _himsg *hip, *workp;
	
	for (hip = hicnts; hip != NULL; hip = hip->hi_next)
		if (strcmp(hip->hi_conf, conf) == 0)
			break;
	if (hip != NULL)
		hip->hi_uns = HI_SUBSCR;
	else {
		if ((workp = (struct _himsg *) calloc((unsigned) 1, sizeof (struct _himsg))) == NULL) {
			log("Error %d allocating _himsg for %s", errno, conf);
			panic("alloc");
		}
		strcpy(workp->hi_conf, conf);
		workp->hi_num = 0;
		workp->hi_next = hicnts;
		hicnts = workp;
		workp->hi_uns = HI_SUBSCR;
	}
	writehigh(hicnts);
	log("Resubscribed to %s", conf);
	writef("Resubscribed to conference %s.\n", conf);
}

unsub() {
	char line[256], *p;

	for (;;) {
		writef("Unsubscribe to which conference (ENTER to abort): ");
		reads(line);
		log("Unsub conference: %s", line);
		if (line[0] == '\0')
			return 1;
		if (isconf(line)) {
			unsubscribe(line);
			return 1;
		}
		writef("That's not a valid conference.  ");
	}
}

isconf(conf)
char *conf; {
	char *cp, line[256];

	for (cp = conf; *cp != 0; cp++) {
		if (!isprint(*cp))
			return 0;
		else if (*cp == '/' || *cp == '!' || *cp == ':')
			*cp = '.';
		else
			*cp = ToLower(*cp);
	}
	if (cp - conf > CONFSIZE)
		conf[CONFSIZE] = '\0';
	sprintf(line, "%s/%s", MSGBASE, conf);
	if (chdir(line) == -1)
		return 0;
	if (chdir(parms.ua_home) == -1) {
		log("Can't chdir to HOME (errno=%d)", errno);
		system("pwd");
		panic("isconf_cd");
	}
	return 1;
}

setlconf() {
	char line[256], *p;

	if (s_cmp(user.u_name, "guest") == 0) {
		log("Guest SET LOGIN CONF denied.");
		writes("GUEST can't set a login conference.");
		return 1;
	}
	do {
		writef("Enter new login conference: ");
		reads(line);
		log("Login conference: %s", line);
		if (line[0] == '\0')
			return 1;
	} while (!isconf(line));
	if (isunsub(line)) {
		writes("You're unsubscribed from it.  <J>oin it and resubscribe.");
		log("Unsubscribed -- login conf set aborted.");
		return 1;
	}
	if (!isrcmem(user.u_name, line)) {
		writes("You aren't a member of that conference.");
		log("Not a member -- login conf set aborted.");
		return 1;
	}
	strcpy(user.u_lconf, line);
	log("New login conference: %s", user.u_lconf);
	putuser(user.u_name, &user);
	return 1;
}

uisunsub(uname, conf)
char *uname, *conf; {
	struct _himsg *hip, *uhi;
	char *cp;

	for (cp = uname; *cp != '\0'; cp++)
		*cp = ToLower(*cp);
	if ((uhi = readhigh(uname)) < 0) {
		log("Couldn't read %s's userindex.", uname);
		return 0;
	}
	writef("Checking %s's user index...\n", uname);
	for (hip = uhi; hip != NULL; hip = hip->hi_next)
		if (strcmp(hip->hi_conf, conf) == 0)
			break;
	cp = (hip != NULL && hip->hi_uns == HI_UNSUB? "!": ":");
	for (hip = uhi; hip != NULL; hip = uhi) {
		uhi = hip->hi_next;
		free((char *) hip);
	}
	return (*cp == '!');
}

cleanhigh() {
	struct _himsg *hip, *lastp;
	DIR *confs;
	struct direct *conf;
	int nunread;
	char line[80];
	FILE *fp;
	
	lastp = NULL;
	writes("Checking for deleted conferences...");
	for (hip = hicnts; hip != NULL; lastp = hip, hip = hip->hi_next) {
		if (!isconf(hip->hi_conf)) {
			writef("Conference \"%s\" was deleted since your last session.\n", hip->hi_conf);
			if (lastp == NULL)
				hicnts = hip->hi_next;
			else
				lastp->hi_next = hip->hi_next;
			free((char *) hip);
		}
	}
	writes("\nChecking for new messages and conferences...");
	if ((confs = opendir(MSGBASE)) == NULL) {
		log("Error %d opening dir %s/", errno, MSGBASE);
		panic("msgdir");
	}
	while ((conf = readdir(confs)) != NULL) {
		if (strcmp(conf->d_name, ".") == 0)
			continue;
		if (strcmp(conf->d_name, "..") == 0)
			continue;
		for (hip = hicnts; hip != NULL; hip = hip->hi_next)
			if (strcmp(hip->hi_conf, conf->d_name) == 0) {
				sprintf(line, "%s/%s/himsg", MSGBASE, conf->d_name);
				if ((fp = fopen(line, "r")) == (FILE *) 0)
					break;
				fgets(line, 32, fp);
				fclose(fp);
				nunread = atoi(line);
				if ((nunread -= hip->hi_num) <= 0)
					break;
				writef("There are %d new messages in \"%s\".\n", nunread, conf->d_name);
				break;
			}
		if (hip == NULL) {
			writef("Conference \"%s\" has been created since your last session.\n", conf->d_name);
			if ((hip = (struct _himsg *) calloc((unsigned) 1, sizeof (struct _himsg))) == NULL) {
				log("Error %d allocating _himsg for %s", errno, conf);
				panic("alloc");
			}
			strcpy(hip->hi_conf, conf->d_name);
			hip->hi_num = 0;
			hip->hi_next = hicnts;
			hicnts = hip;
		}
	}
	writehigh(hicnts);
	closedir(confs);
}

cnread(conf)
char *conf; {
	struct _himsg *hi;
	
	for (hi = hicnts; hi != (struct _himsg *) 0; hi = hi->hi_next)
		if (s_cmp(conf, hi->hi_conf) == 0)
			return hi->hi_num;
	return -1;
}

edrest() {
	char rconf[256], line[256];
	char *p;
	FILE *fp;

	if (user.u_access != A_WITNESS) {
		writes("You aren't permitted to edit Restricted conference membership lists.");
		log("Non-FW attempted to edit restricted conf membership lists");
		return 1;
	}
	if (!parms.ua_xrc) {
		writes("Restricted conferences are not permitted on this BBS.");
		log("redit: restricted conferences disabled");
		return 1;
	}
	writef("Enter conference (RETURN / ENTER to abort): ");
	reads(rconf);
	log("Rconf: %s", rconf);
	if (rconf[0] == '\0')
		return 1;
	if (!isconf(rconf)) {
		writef("Conference \"%s\" doesn't exist.", rconf);
		log("Bad conference: %s", rconf);
		return 1;
	}
	for (p = rconf; *p != '\0'; p++)
		*p = ToLower(*p);
	if (s_cmp(user.u_name, parms.ua_sysop) != 0 && !isrcmem(user.u_name, rconf)) {
		log("FW not a member; list only");
		rcmemlist(rconf);
		return 1;
	}
	if (!isrconf(rconf))
		if (s_cmp(user.u_name, parms.ua_sysop) != 0) {
			writes("Only the Sysop can restrict a conference's membership.");
			return 1;
		}
		else {
			writef("Conference \"%s\" isn't restricted.  Restrict? N\b", rconf);
			line[0] = readc();
			log("Restrict %s? %c", rconf, line[0]);
			if (line[0] != 'Y')
				return 1;
			sprintf(line, "%s/%s", MEMLIST, rconf);
			if ((fp = fopen(line, "w")) == (FILE *) 0) {
				log("Error %d creating %s", errno, line);
				panic("rest_mk");
			}
			fclose(fp);
		}
	for (;;) {
		writef("\nList members, Add a member, Delete a member, Clear membership list, or Quit: ");
		line[0] = readc();
		switch (line[0]) {
		case 'L':
			rcmemlist(rconf);
			break;
		case 'A':
			rcmemadd(rconf);
			break;
		case 'D':
			rcmemdel(rconf);
			break;
		case 'C':
			rcmemclr(rconf);
			break;
		case 'Q':
			return 1;
		default:
			writes("Please enter L, A, D, C, or Q.");
		}
	}
}

isrcmem(uname, conf)
char *uname, *conf; {
	FILE *fp;
	char line[256];
	char *cp;

	if (!parms.ua_xrc || s_cmp(uname, parms.ua_sysop) == 0)
		return 1;
	sprintf(line, "%s/%s", MEMLIST, conf);
	if ((fp = fopen(line, "r")) == (FILE *) 0)
		return 1;	/* no mem list == no restrictions */
	while (fgets(line, sizeof line, fp) != (char *) 0) {
		if ((cp = RIndex(line, '\n')) != (char *) 0)
			*cp = '\0';
		if (s_cmp(line, uname) == 0) {
			fclose(fp);
			log("%s is a member of restricted conf %s", uname, conf);
			return 1;
		}
	}
	fclose(fp);
	log("%s isn't a member of restricted conf %s", uname, conf);
	return 0;
}

rcmemlist(conf)
char *conf; {
	FILE *fp;
	char line[256];
	short headf;

	if (!parms.ua_xrc) {
		writef("Conference \"%s\" has no restrictions on membership.\n", conf);
		return;
	}
	sprintf(line, "%s/%s", MEMLIST, conf);
	if ((fp = fopen(line, "r")) == (FILE *) 0) {
		writef("Conference \"%s\" has no restrictions on membership.\n", conf);
		return;
	}
	headf = 0;
	while (fgets(line, sizeof line, fp) != (char *) 0) {
		if (!headf) {
			writef("Members of the \"%s\" conference:\n\n", conf);
			headf++;
		}
		writef("\t%s\n", upstr(line));
		/* OOPS!  1 line deleted 2/3/87 ++bsa */
	}
	if (!headf)
		writef("Conference \"%s\" is restricted to Fairwitnesses and the Sysop.\n", conf);
	fclose(fp);
}

rcmemadd(conf)
char *conf; {
	char line[256], uname[256];
	struct user ubuf;
	FILE *fp;
	
	writef("Name (RETURN to abort): ");
	reads(uname);
	log("Add user %s to %s's mem list", uname, conf);
	if (uname[0] == '\0')
		return;
	if (!getuser(uname, &ubuf)) {
		writef("User \"%s\" doesn't exist.\n", upstr(uname));
		log("No such user: %s", uname);
		return;
	}
	if (ubuf.u_access == A_WITNESS && s_cmp(user.u_name, "sysop") != 0) {
		log("FW attempted to change membership of %s in %s", ubuf.u_name, conf);
		writes("Sorry, only the Sysop can change a FairWitness's conference membership.");
		return;
	}
	if (isrcmem(uname, conf)) {
		log("Already a member.");
		writef("\"%s\" is already a member of this conference.\n", upstr(uname));
		return;
	}
	mklock("memlist.lock");
	sprintf(line, "%s/%s", MEMLIST, conf);
	if ((fp = fopen(line, "a")) == (FILE *) 0) {
		rmlock(conf);
		log("Error %d opening %s", errno, line);
		panic("memlist_app");
	}
	fprintf(fp, "%s\n", upstr(uname));	/* OOPS!  2/3/87 ++bsa */
	fclose(fp);
	rmlock("memlist.lock");
}

rcmemdel(conf)
char *conf; {
	char line[256], uname[256], tname[256];
	struct user ubuf;
	FILE *fp, *tp;
	
	writef("Name (RETURN to abort): ");
	reads(uname);
	log("Del user %s from %s's mem list", uname, conf);
	if (uname[0] == '\0')
		return;
	if (!getuser(uname, &ubuf)) {
		writef("User \"%s\" doesn't exist.\n", upstr(uname));
		log("No such user: %s", uname);
		return;
	}
	if (ubuf.u_access == A_WITNESS && s_cmp(user.u_name, "sysop") != 0) {
		log("FW attempted to change membership of %s in %s", ubuf.u_name, conf);
		writes("Sorry, only the Sysop can change a FairWitness's conference membership.");
		return;
	}
	if (!isrcmem(uname, conf)) {
		log("Not a member.");
		writef("\"%s\" isn't a member of this conference.\n", upstr(uname));
		return;
	}
	sprintf(tname, "/tmp/UAxD%05d", getpid());
	if ((tp = fopen(tname, "w")) == (FILE *) 0) {
		log("Error %d opening %s", errno, tname);
		panic("memlist_dtmp");
	}
	mklock("memlist.lock");
	sprintf(line, "%s/%s", MEMLIST, conf);
	if ((fp = fopen(line, "r")) == (FILE *) 0) {
		rmlock(conf);
		fclose(tp);
		unlink(tname);
		log("Error %d opening %s", errno, line);
		panic("memlist_app");
	}
	while (fgets(line, sizeof line, fp) != (char *) 0)
		if (s_cmp(line, uname) != 0)
			fputs(line, tp);
	fclose(fp);
	fclose(tp);
	sprintf(line, "%s/%s", MEMLIST, conf);
	if (unlink(line) < 0) {
		log("Error %d unlinking %s", errno, line);
		rmlock("memlist.lock");
		panic("memlist_drmv");
	}
	if (copylink(tname, line) < 0) {
		log("Error %d copylinking %s to %s", errno, tname, line);
		rmlock("memlist.lock");
		panic("memlist_dclnk");
	}
	rmlock("memlist.lock");
}

rcmemclr(conf)
char *conf; {
	char mlist[256];
	
	if (s_cmp(user.u_name, parms.ua_sysop) != 0) {
		log("Attempt to clear %s's mem list by non-sysop", conf);
		writes("Only the Sysop can clear a conference's membership list.");
		return;
	}
	writef("Clear membership list for the \"%s\" conference? N\b", conf);
	mlist[0] = readc();
	log("Clear? %c", mlist[0]);
	if (mlist[0] != 'Y') {
		log("Aborted.");
		writes("Aborted.");
		return;
	}
	sprintf(mlist, "%s/%s", MEMLIST, conf);
	if (unlink(mlist) < 0) {
		log("Error %s unlinking %s", errno, mlist);
		writes("Can't remove the membership list.");
	}
}

isrconf(rconf)
char *rconf; {
	char line[256];
	FILE *fp;
	
	sprintf(line, "%s/%s", MEMLIST, rconf);
	if ((fp = fopen(line, "r")) == (FILE *) 0)
		return 0;
	fclose(fp);
	return 1;
}
