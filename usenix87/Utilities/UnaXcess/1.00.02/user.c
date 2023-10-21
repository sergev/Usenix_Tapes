/*
 *	@(#)user.c	1.2 (TDI) 2/3/87
 *	@(#)Copyright (C) 1984, 85, 86, 87 by Brandon S. Allbery.
 *	@(#)This file is part of UNaXcess version 1.0.2.
 *
 *	Permission is hereby granted to copy and distribute this program
 *	freely.  Permission is NOT given to modify this program or distribute
 *	it at cost, except for charging a reasonable media/copying fee.
 */

#ifndef lint
static char _FileID_[] = "@(#)user.c	1.2 (TDI) 2/3/87";
static char _UAID_[]   = "@(#)UNaXcess version 1.0.2";
#endif lint

#include "ua.h"

struct user user;
struct _himsg *hicnts;

getuser(name, buf)
char *name;
struct user *buf; {
	FILE *bfd;
	char line[1024], lcuname[33], *p, *q;
	int ncolon;

	if ((bfd = fopen(PASSWD, "r")) == NULL) {
		log("Error %d opening %s", errno, PASSWD);
		panic("passwd");
	}
	for (p = name, q = lcuname; *p != '\0' && p - name <= 32; p++, q++)
		*q = ToLower(*p);
	*q = '\0';
	while (fgets(line, sizeof line, bfd) != NULL)
		if (strncmp(line, lcuname, strlen(lcuname)) == 0 && line[strlen(lcuname)] == ':') {
			fclose(bfd);
			buf->u_name[0] = '\0';
			buf->u_pass[0] = '\0';
			buf->u_access = A_NONE;
			buf->u_login[0] = '\0';
			buf->u_llen = 32;
			buf->u_nbull = 0;
			strcpy(buf->u_lconf, "general");
			buf->u_lines = 16;
			ncolon = 0;
			for (p = line; *p != '\0'; p++)
				if (*p == ':')
					ncolon++;
			if (ncolon < 5) {
				log("Bad userfile entry for %s", lcuname);
				return 0;
			}
			for (p = line, q = buf->u_name; *p != ':'; p++, q++)
				*q = *p;
			*q = '\0';
			for (p++, q = buf->u_pass; *p != ':'; p++, q++)
				*q = *p;
			*q = '\0';
			for (p++, q = lcuname; *p != ':'; p++, q++)
				*q = *p;
			*q = '\0';
			buf->u_access = atoi(lcuname);
			for (p++, q = buf->u_login; *p != ':'; p++, q++)
				*q = *p;
			*q = '\0';
			for (p++, q = lcuname; *p != ':'; p++, q++)
				*q = *p;
			*q = '\0';
			buf->u_llen = atoi(lcuname);
			for (p++, q = lcuname; *p != ':'; p++, q++)
				*q = *p;
			*q = '\0';
			buf->u_nbull = atoi(lcuname);
			if (ncolon >= 6) {
				for (p++, q = buf->u_lconf; *p != '\n' && *p != ':'; p++, q++)
					*q = *p;
				*q = '\0';
			}
			if (ncolon >= 7) {
				for (p++, q = lcuname; *p != '\n'; p++, q++)
					*q = *p;
				*q = '\0';
				buf->u_lines = atoi(lcuname);
			}
			return 1;
		}
	fclose(bfd);
	return 0;
}

putuser(name, ubuf)
char *name;
struct user *ubuf; {
	FILE *fd, *tfd;
	char line[1024], *tempfile = mktemp("/tmp/UptXXXXXX"), lcname[33], *p, *q;
	static char lockfile[] = "userfil.lock";
	short flag;

	if (s_cmp(user.u_name, "guest") == 0)
		return;
	CRIT();
	mklock(lockfile);
	if ((fd = fopen(PASSWD, "r")) == NULL) {
		log("Error %d opening %s", errno, PASSWD);
		panic("passwd");
	}
	if ((tfd = fopen(tempfile, "w")) == NULL) {
		log("Error %d opening %s", errno, tempfile);
		panic("tmp");
	}
	flag = 0;
	for (p = name, q = lcname; *p != '\0' && p - name < 33; p++, q++)
		*q = ToLower(*p);
	*q = 0;
	while (fgets(line, sizeof line, fd) != NULL)
		if (strncmp(line, lcname, strlen(lcname)) == 0 && line[strlen(lcname)] == ':') {
			if (ubuf != (struct user *) 0)	/* not deleting */
				fprintf(tfd, "%s:%s:%d:%s:%d:%d:%s:%d\n", ubuf->u_name, ubuf->u_pass, ubuf->u_access, ubuf->u_login, ubuf->u_llen, ubuf->u_nbull, ubuf->u_lconf, ubuf->u_lines);
			flag++;
		}
		else
			fputs(line, tfd);
	if (!flag && ubuf != (struct user *) 0)
		fprintf(tfd, "%s:%s:%d:%s:%d:%d:%s:%d\n", ubuf->u_name, ubuf->u_pass, ubuf->u_access, ubuf->u_login, ubuf->u_llen, ubuf->u_nbull, ubuf->u_lconf, ubuf->u_lines);
	fclose(fd);
	fclose(tfd);
	unlink(PASSWD);
	if (copylink(tempfile, PASSWD) < 0) {
		log("Error %d copylinking %s to %s", errno, tempfile, PASSWD);
		panic("copylink");
	}
	unlink(tempfile);
	rmlock(lockfile);
	NOCRIT();
}

writehigh(hilist)
struct _himsg *hilist; {
	FILE *hp, *f;
	static char line[1024], hirec[1024];	/* 68000's have limited frames */
	char *tmpf = mktemp("/tmp/RcXXXXXX");
	char *eofflag;
	static char lockfile[] = "newmsgs.lock";
	struct _himsg *hptr;

	if (s_cmp(user.u_name, "guest") == 0)
		return;	/* don't write GUEST hirecs! */
	CRIT();
	if ((f = fopen(tmpf, "w")) == NULL) {
		log("Error %d opening %s", errno, tmpf);
		panic("tmp");
	}
	if ((hp = fopen(NEWMSGS, "r")) == NULL) {
		log("Error %d opening %s", errno, NEWMSGS);
		fclose(f);
		unlink(tmpf);
		panic("userind");
	}
	mklock(lockfile);
	sprintf(line, "%s:", user.u_name);
	while ((eofflag = fgets(hirec, sizeof hirec, hp)) != NULL) {
		if (strncmp(hirec, line, strlen(line)) == 0)
			break;
		fputs(hirec, f);
	}
	if (eofflag != NULL)
		while ((eofflag = fgets(hirec, sizeof hirec, hp)) != NULL)
			if (hirec[0] != '\t' && hirec[0] != ' ')
	   			break;
	fputs(line, f);
	putc('\n', f);
	for (hptr = hilist; hptr != NULL; hptr = hptr->hi_next)
		fprintf(f, "\t%s%c %d\n", hptr->hi_conf, (hptr->hi_uns == HI_UNSUB? '!': ':'), hptr->hi_num);
	if (eofflag != NULL && hirec[0] != '\t' && hirec[0] != ' ')
		fputs(hirec, f);
	if (eofflag != NULL)
		while (fgets(hirec, sizeof hirec, hp) != NULL)
			fputs(hirec, f);
	fclose(f);
	fclose(hp);
	unlink(NEWMSGS);
	if (copylink(tmpf, NEWMSGS) < 0) {
		log("Error %d copylinking %s to %s", errno, tmpf, NEWMSGS);
		panic("copylink");
	}
	unlink(tmpf);
	rmlock(lockfile);
	NOCRIT();
}

struct _himsg *readhigh(foruser)
struct user *foruser; {
	static char hirec[1024];
	char uidx[40], *p, *q;
	FILE *f;
	struct _himsg *workp, *initp;

	strcpy(uidx, foruser->u_name);
	strcat(uidx, ":");
	if ((f = fopen(NEWMSGS, "r")) == NULL)
		return NULL;
	while (fgets(hirec, sizeof hirec, f) != NULL)
		if (strncmp(hirec, uidx, strlen(uidx)) == 0)
			break;
	if (feof(f)) {
		fclose(f);
		return NULL;
	}
	workp = NULL;
	initp = NULL;
	while (fgets(hirec, sizeof hirec, f) != NULL && (hirec[0] == ' ' || hirec[0] == '\t')) {
		hirec[strlen(hirec) - 1] = '\0';
		for (p = hirec; *p == ' ' || *p == '\t'; p++)
			;
		for (q = uidx; *p != ' ' && *p != '\t' && *p != '\0' && *p != ':' && *p != '!'; p++)
			*q++ = *p;
		*q = '\0';
		while (*p == ' ' || *p == '\t')
			p++;
		if (*p == '!') {	/* unsubscribed... */
			if ((workp = (struct _himsg *) calloc((unsigned) 1, sizeof (struct _himsg))) == NULL) {
				log("Error %d allocating _himsg for %s", errno, uidx);
				panic("alloc");
			}
			strcpy(workp->hi_conf, uidx);
			workp->hi_num = atoi(++p);
			workp->hi_next = initp;
			workp->hi_uns = HI_UNSUB;
			initp = workp;
			continue;
		}
		if (*p != ':') {
			log("Invalid format of userind record: ``%s''", hirec);
			writes("Your index is garbled; some conference\nhigh-message counts may be lost.");
			break;
		}
		if ((workp = (struct _himsg *) calloc((unsigned) 1, sizeof (struct _himsg))) == NULL) {
			log("Error %d allocating _himsg for %s", errno, uidx);
			panic("alloc");
		}
		strcpy(workp->hi_conf, uidx);
		workp->hi_num = atoi(++p);
		workp->hi_next = initp;
		workp->hi_uns = HI_SUBSCR;
		initp = workp;
	}
	fclose(f);
	return initp;
}

newuser() {
	struct user nubuf, junk;
	char line[256], addr[256], cityst[256], phone[256], uname[256], ckpass[256];
	char *p;
	FILE *newp;

	dolog();
	log("Entered newuser module.");
	cat(NEWUSER);
	writef("\nDo you still want to become a user? N\b");
	line[0] = readc();
	log("Become user? %c", line[0]);
	if (line[0] != 'Y') {
		writes("OK.  Goodbye, then.\n\n");
		nolog();
		cleanup();
	}

Again:
	do {
		writef("What name would you like to use on this system?  It should not be\nmore than 32 letters long: ");
		reads(line);
		log("Name: %s", line);
		if (line[0] == '\0' || line[0] == ' ') {
			line[0] = '?';
			p = line;
			continue;
		}
		for (p = line; *p != '\0'; p++)
			if (*p == ':') {
				writes("Sorry, no colons allowed; they cause nasty surprises.");
				log("Illegal colon in name");
				break;
			}
	} while (*p != NULL);
	if (getuser(nubuf.u_name, &junk)) {
		writes("Sorry, but that name's already in use.  Please choose another.");
		goto Again;
	}
	strncpy(nubuf.u_name, line, 32);
	nubuf.u_name[32] = '\0';
	writef("Please enter your street address: ");
	reads(addr);
	writef("Please enter your city and state: ");
	reads(cityst);
	writef("Please enter your home phone number: ");
	reads(phone);
	line[0] = '\0';
	xecho();
	do {
		if (line[0] != 0)
			writes("You made a typing error, or it's too short or too long.");
		writef("Please enter a password of three to eight characters.\nIt will not be displayed: ");
		reads(line);
		log("Pass: %s", line);
		writef("Please re-enter it, just to make sure: ");
		reads(ckpass);
		log("Ckpass: %s", ckpass);
	} while (strlen(line) < 3 || strlen(line) > 8 || strcmp(line, ckpass) != 0);
	doecho();
	strcpy(nubuf.u_pass, line);
	do {
		writef("How many characters per line are on your terminal?  Please enter a number from 32 to 132, or <ENTER> for 80: ");
		reads(line);
		log("Line: %s", line);
		if (line[0] == '\0')
			nubuf.u_llen = 80;
		else
			nubuf.u_llen = atoi(line);
	} while (nubuf.u_llen < 32 || nubuf.u_llen > 132);
	do {
		writef("How many lines are on your terminal?  Please enter a number from 0 to 66, or <ENTER> for 24: ");
		reads(line);
		log("Line: %s", line);
		if (line[0] == '\0')
			nubuf.u_lines = 24;
		else
			nubuf.u_lines = atoi(line);
	} while (nubuf.u_lines < 0 || nubuf.u_lines > 66);
	writef("If you are a regular (shell) user of this system, please enter your login name.  The sysop will verify it and grant you SYSTEM-level access.  If you are not a regular user, just press RETURN (ENTER).\n\nWhat is your Unix(R) login name on this sy

stem? ");
	reads(uname);
	writef("\nName:\t\t%s\nAddress:\t%s\nCity/State:\t%s\nPhone:\t\t%s\nPass:\t\t%s\nLine:\t\t%d\nScreen:\t\t%d\nLogname:\t%s\n\nIs this correct? N\b", nubuf.u_name, addr, cityst, phone, nubuf.u_pass, nubuf.u_llen, nubuf.u_lines, uname);
	line[0] = readc();
	log("Okay? %c", line[0]);
	if (line[0] != 'Y')
		goto Again;
	writes("Encrypting password, please wait...");
	strcpy(nubuf.u_pass, crypt(nubuf.u_pass, nubuf.u_pass) + 2);
	strcpy(nubuf.u_login, user.u_login);/* default login name ( guest ?) */
	nubuf.u_access = parms.ua_vaxs;
	nubuf.u_nbull = 0;			/* no bulletins read yet */
	strcpy(nubuf.u_lconf, "general");
	writes("Recording user information...");
	for (p = nubuf.u_name; *p != '\0'; p++)
		*p = ToLower(*p);
	putuser(nubuf.u_name, &nubuf);
	CRIT();
	mklock("newuser.lock");
	if ((newp = fopen("newuser.log", "a")) == (FILE *) 0) {
		log("Error %d opening newuser.log", errno);
		panic("newlog");
	}
	fprintf(newp, "%s:%s:%s:%s:%s\n", nubuf.u_name, addr, cityst, phone, uname);
	fclose(newp);
	rmlock("newuser.lock");
	NOCRIT();
	user = nubuf;
	nolog();
}

userlist() {
	FILE *bfd, *wfd;
	char line[1024], *p, *q, dbuf[20];
	short ncolon;
	struct user buf;

	if ((bfd = fopen(PASSWD, "r")) == NULL) {
		log("Error %d opening %s", errno, PASSWD);
		panic("passwd");
	}
	if (user.u_access == A_WITNESS)
		wfd = fopen("newuser.log", "r");
	else
		wfd = (FILE *) 0;
	writes("\nList of UNaXcess users:\n");
	while (fgets(line, 1024, bfd) != NULL) {
		buf.u_name[0] = '\0';
		buf.u_pass[0] = '\0';
		buf.u_access = 0;
		buf.u_login[0] = '\0';
		buf.u_llen = 0;
		buf.u_nbull = 0;
		buf.u_lconf[0] = '\0';
		ncolon = 0;
		for (p = line; *p != '\0'; p++)
			if (*p == ':')
				ncolon++;
		if (ncolon < 5) {
			log("Bad usefile entry %.*s", strlen(line) - 1, line);
			continue;
		}
		for (p = line, q = buf.u_name; *p != ':'; p++, q++)
			*q = *p;
		*q = '\0';
		for (p++, q = buf.u_pass; *p != ':'; p++, q++)
			*q = *p;
		*q = '\0';
		for (p++, q = dbuf; *p != ':'; p++, q++)
			*q = *p;
		*q = '\0';
		buf.u_access = atoi(dbuf);
		for (p++, q = buf.u_login; *p != ':'; p++, q++)
			*q = *p;
		*q = '\0';
		for (p++, q = dbuf; *p != ':'; p++, q++)
			*q = *p;
		*q = '\0';
		buf.u_llen = atoi(dbuf);
		for (p++, q = dbuf; *p != ':'; p++, q++)
			*q = *p;
		*q = '\0';
		buf.u_nbull = atoi(dbuf);
		if (ncolon >= 6) {
			for (p++, q = buf.u_lconf; *p != '\n'; p++, q++)
				*q = *p;
			*q = '\0';
		}
		if (buf.u_access == A_NONE || buf.u_access == A_MKUSER)
			continue;
		writef("%-32.32s  %s\n", upstr(buf.u_name), ua_acl(buf.u_access));
		if (wfd != (FILE *) 0) {
			rewind(wfd);
			do {
				if (fgets(line, sizeof line, wfd) == (char *) 0) {
					buf.u_name[0] = '\0';
					break;
				}
			} while (strncmp(line, buf.u_name, strlen(buf.u_name)) != 0 || line[strlen(buf.u_name)] != ':');
			if (buf.u_name[0] == '\0')
				continue;
			p = &line[strlen(buf.u_name) + 1];
			for (q = p; *q != ':'; q++)
				;
			*q++ = '\0';
			writef("\t%s\n", p);
			p = q;
			for (q = p; *q != ':'; q++)
				;
			*q++ = '\0';
			writef("\t%s\n", p);
			p = q;
			for (q = p; *q != ':'; q++)
				;
			*q++ = '\0';
			writef("\tPhone: %s\n\n", p);
		}
	}
	fclose(bfd);
	return 1;
}

char *ua_acl(acl) {
	switch (acl) {
	case A_NONE:
		return "Denied access";
	case A_GUEST:
		return "Guest";
	case A_USER:
		return "Messages";
	case A_FILES:
		return "File UDL";
	case A_SYSTEM:
		return "System";
	case A_WITNESS:
		return "Fairwitness";
	case A_MKUSER:
		return "New User";
	default:
		log("Invalid access level %d in userfile", acl);
		return "???";
	}
}

validate() {
	FILE *bfd, *wfd, *vfd;
	struct user buf;
	char line[1024], dbuf[20];
	short ncolon;
	char *p, *q, *vfile;
	char ch;

	if (user.u_access != A_WITNESS) {
		log("Non-Witness attempted to validate users.");
		writes("You aren't authorized to validate users.\n");
		return 1;
	}
	if ((bfd = fopen(PASSWD, "r")) == NULL) {
		log("Error %d opening %s", errno, PASSWD);
		panic("passwd");
	}
	if ((wfd = fopen("newuser.log", "r")) == (FILE *) 0) {
		log("Error %d opening newuser.log", errno);
		panic("nuserlog");
	}
	vfile = mktemp("/tmp/uaVXXXXXX");
	if ((vfd = fopen(vfile, "w")) == (FILE *) 0) {
		log("Error %d opening %s", errno, vfile);
		panic("valtemp");
	}
	while (fgets(line, sizeof line, bfd) != NULL) {	/* OOPS!  2/3/87 ++bsa */
		buf.u_name[0] = '\0';
		buf.u_pass[0] = '\0';
		buf.u_access = 0;
		buf.u_login[0] = '\0';
		buf.u_llen = 0;
		buf.u_nbull = 0;
		buf.u_lconf[0] = '\0';
		ncolon = 0;
		for (p = line; *p != '\0'; p++)
			if (*p == ':')
				ncolon++;
		if (ncolon < 5) {
			log("Bad userfile entry %.*s", strlen(line) - 1, line);
			continue;
		}
		for (p = line, q = buf.u_name; *p != ':'; p++, q++)
			*q = *p;
		*q = '\0';
		for (p++, q = buf.u_pass; *p != ':'; p++, q++)
			*q = *p;
		*q = '\0';
		for (p++, q = dbuf; *p != ':'; p++, q++)
			*q = *p;
		*q = '\0';
		buf.u_access = atoi(dbuf);
		for (p++, q = buf.u_login; *p != ':'; p++, q++)
			*q = *p;
		*q = '\0';
		for (p++, q = dbuf; *p != ':'; p++, q++)
			*q = *p;
		*q = '\0';
		buf.u_llen = atoi(dbuf);
		for (p++, q = dbuf; *p != ':'; p++, q++)
			*q = *p;
		*q = '\0';
		buf.u_nbull = atoi(dbuf);
		if (ncolon >= 6) {
			for (p++, q = buf.u_lconf; *p != '\n'; p++, q++)
				*q = *p;
			*q = '\0';
		}
		if (buf.u_access != parms.ua_vaxs)
			continue;
		rewind(wfd);
		do {
			if (fgets(line, sizeof line, wfd) == (char *) 0) {
				buf.u_name[0] = '\0';
				break;
			}
		} while (strncmp(line, buf.u_name, strlen(buf.u_name)) != 0 || line[strlen(buf.u_name)] != ':');
		if (buf.u_name[0] == '\0')
			continue;
		writef("\n%-32.32s  %s\n", upstr(buf.u_name), ua_acl(buf.u_access));
		writef("\tUNIX(R) name: `%s'\n\n", buf.u_login);
		p = &line[strlen(buf.u_name) + 1];
		for (q = p; *q != ':'; q++)
			;
		*q++ = '\0';
		writef("\t%s\n", p);
		p = q;
		for (q = p; *q != ':'; q++)
			;
		*q++ = '\0';
		writef("\t%s\n", p);
		p = q;
		for (q = p; *q != ':'; q++)
			;
		*q++ = '\0';
		writef("\tPhone: %s\n", p);
		log("Validating %s", buf.u_name);

vcmdloop:
		writef("\nValidate, Kill, Deny access, Hold, Shell, Quit? ");
		ch = readc();
		log("V/K/D/H/S/Q? %c", ch);
		switch (ch) {
		case 'V':
			writef("Validate for: Guest, Messages, Files, System, Witness? ");
			ch = readc();
			log("Access? %c", ch);
			switch (ch) {
			case 'G':
				buf.u_access = A_GUEST;
				break;
			case 'M':
				buf.u_access = A_USER;
				break;
			case 'F':
				buf.u_access = A_FILES;
				break;
			case 'S':
				buf.u_access = A_SYSTEM;
				break;
			case 'W':
				buf.u_access = A_WITNESS;
				break;
			default:
				writes("Unknown access level -- command aborted.");
				goto vcmdloop;
			}
			fprintf(vfd, "%s:%d\n", buf.u_name, buf.u_access);
			break;
		case 'K':
			writef("Kill user? N\b");
			if (readc() != 'Y')
				goto vcmdloop;
			log("User killed.");
			fprintf(vfd, "%s:-1\n", buf.u_name);
			break;
		case 'D':
			log("Access set to A_NONE.");
			buf.u_access = A_NONE;
			fprintf(vfd, "%s:%d\n", buf.u_name, A_NONE);
			break;
		case 'H':
			log("User held.");
			break;
		case 'S':
			shell();
			goto vcmdloop;
		case 'Q':
			fclose(bfd);
			fclose(wfd);
			fclose(vfd);
			vinstall(vfile);
			unlink(vfile);
			return 1;
		default:
			writes("Please enter one of V, K, D, H, S, or Q.");
			goto vcmdloop;
		}
	}
	if (ferror(bfd))
		log("Error %d on userfile\n", errno);
	fclose(bfd);
	fclose(wfd);
	fclose(vfd);
	vinstall(vfile);
	unlink(vfile);
	return 1;
}

vinstall(vfile)
char *vfile; {
	FILE *vfd;
	struct user vuser;
	char vline[1024], vuname[33];
	int vaccess;
	
	writec('\n');
	if ((vfd = fopen(vfile, "r")) == (FILE *) 0) {
		log("Error %d opening %s", errno, vfile);
		writes("Can't open validation file.");
		return;
	}
	while (fgets(vline, sizeof vline, vfd) != (char *) 0) {
		sscanf(vline, "%[^:]:%d", vuname, &vaccess);
		if (!getuser(vuname, &vuser)) {
			writef("Lost user: %s\n", upstr(vuname));
			continue;
		}
		if (vaccess == -1) {
			putuser(vuname, (struct user *) 0);
			writef("Killed user %s...\n", upstr(vuname));
			continue;
		}
		vuser.u_access = vaccess;
		putuser(vuname, &vuser);
		if (vaccess == A_NONE)
			writef("Denied access to user %s...\n", upstr(vuname));
		else
			writef("Validated user %s for %s access...\n", upstr(vuname), ua_acl(vaccess));
	}
	fclose(vfd);
}
