/*
 *	@(#)user.c	1.1 (TDI) 7/18/86 21:02:04
 *
 * Modification History:
 *
 *   Ver. 1.1 created 07/18/86 at 21:02:04 by brandon
 *     Converted to SCCS 07/18/86 21:02:03
 *	@(#)Copyright (C) 1984, 85, 86 by Brandon S. Allbery.
 *
 * Modification History:
 *
 *   Ver. 1.1 created 07/18/86 at 21:02:04 by brandon
 *     Converted to SCCS 07/18/86 21:02:03
 *
 *	@(#)This file is part of UNaXcess version 0.4.5.
 *
 * Modification History:
 *
 *   Ver. 1.1 created 07/18/86 at 21:02:04 by brandon
 *     Converted to SCCS 07/18/86 21:02:03
 *
 *	Permission is hereby granted to copy and distribute this program
 *	freely.  Permission is NOT given to modify this program or distribute
 *	it at cost, except for charging a reasonable media/copying fee.
 */

#ifndef lint
static char _FileID_[] = "@(#)user.c	1.1 (TDI) 7/18/86 21:02:04";
static char _UAID_[]   = "@(#)UNaXcess version 0.4.5";
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
			buf->u_access = 0;
			buf->u_login[0] = '\0';
			buf->u_llen = 0;
			buf->u_nbull = 0;
			buf->u_lconf[0] = '\0';
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
			for (p++, q = buf->u_lconf; *p != '\n'; p++, q++)
				*q = *p;
			*q = '\0';
			return 1;
		}
	fclose(bfd);
	return 0;
}

putuser(name, ubuf)
    char *name;
    struct user *ubuf;
    {
    FILE *fd, *tfd;
    char line[1024], *tempfile = mktemp("/tmp/UptXXXXXX"), lcname[33], *p, *q;
    static char lockfile[] = "userfil.lock";
    short flag;

    CRIT();
    mklock(lockfile);
    if ((fd = fopen(PASSWD, "r")) == NULL)
	{
	log("Error %d opening %s", errno, PASSWD);
	panic("passwd");
	}
    if ((tfd = fopen(tempfile, "w")) == NULL)
	{
	log("Error %d opening %s", errno, tempfile);
	panic("tmp");
	}
    flag = 0;
    for (p = name, q = lcname; *p != '\0' && p - name < 33; p++, q++)
	*q = ToLower(*p);
    *q = 0;
    while (fgets(line, sizeof line, fd) != NULL)
	if (strncmp(line, lcname, strlen(lcname)) == 0 && line[strlen(lcname)] == ':')
	    {
	    fprintf(tfd, "%s:%s:%d:%s:%d:%d:%s\n", ubuf->u_name, ubuf->u_pass, ubuf->u_access, ubuf->u_login, ubuf->u_llen, (s_cmp(ubuf->u_name, "guest") == 0? 0: ubuf->u_nbull), (s_cmp(ubuf->u_name, "guest") == 0? "": ubuf->u_lconf));
	    flag++;
	    }
	else
	    fputs(line, tfd);
    if (!flag)
	fprintf(tfd, "%s:%s:%d:%s:%d:%d:%s\n", ubuf->u_name, ubuf->u_pass, ubuf->u_access, ubuf->u_login, ubuf->u_llen, (s_cmp(ubuf->u_name, "guest") == 0? 0: ubuf->u_nbull), (s_cmp(ubuf->u_name, "guest") == 0? "": ubuf->u_lconf));
    fclose(fd);
    fclose(tfd);
    unlink(PASSWD);
    if (copylink(tempfile, PASSWD) < 0)
	{
	log("Error %d copylinking %s to %s", errno, tempfile, PASSWD);
	panic("copylink");
	}
    unlink(tempfile);
    rmlock(lockfile);
    NOCRIT();
    }

writehigh(hilist)
    struct _himsg *hilist;
    {
    FILE *hp, *f;
    static char line[1024], hirec[1024];	/* 68000's have limited frames */
    char *tmpf = mktemp("/tmp/RcXXXXXX");
    char *eofflag;
    static char lockfile[] = "newmsgs.lock";
    struct _himsg *hptr;

    if (s_cmp(user.u_name, "guest") == 0)
        return;	/* don't write GUEST hirecs! */
    CRIT();
    if ((f = fopen(tmpf, "w")) == NULL)
	{
	log("Error %d opening %s", errno, tmpf);
	panic("tmp");
	}
    if ((hp = fopen(NEWMSGS, "r")) == NULL)
	{
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
    if (copylink(tmpf, NEWMSGS) < 0)
	{
	log("Error %d copylinking %s to %s", errno, tmpf, NEWMSGS);
	panic("copylink");
	}
    unlink(tmpf);
    rmlock(lockfile);
    NOCRIT();
    }

struct _himsg *readhigh(foruser)
    struct user *foruser;
    {
    static char hirec[1024];
    char uidx[40], *p, *q;
    FILE *f;
    struct _himsg *workp, *initp, *lastp;

    strcpy(uidx, foruser->u_name);
    strcat(uidx, ":");
    if ((f = fopen(NEWMSGS, "r")) == NULL)
	return NULL;
    while (fgets(hirec, sizeof hirec, f) != NULL)
	if (strncmp(hirec, uidx, strlen(uidx)) == 0)
	    break;
    if (feof(f))
	{
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
	    if ((workp = (struct _himsg *) calloc((unsigned) 1, sizeof (struct _himsg))) == NULL)
	        {
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
	    puts("Your index is garbled; some conference\nhigh-message counts may be lost.");
	    break;
	}
	if ((workp = (struct _himsg *) calloc((unsigned) 1, sizeof (struct _himsg))) == NULL)
	    {
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

newuser()
    {
    struct user nubuf, junk;
    char line[256], *p;

    log("Entered newuser module.");
    cat(NEWUSER);

Again:
    printf("\nDo you still want to become a user (N)? ");
    gets(line);
    log("Become user? %s", line);
    if (ToLower(line[0]) != 'y')
	return;
    do
	{
	printf("What name would you like to use on this system?  It should not be\nmore than 32 letters long: ");
	gets(line);
	log("Name: %s", line);
	if (line[0] == '\0' || line[0] == ' ')
	    {
	    line[0] = '?';
	    p = line;
	    continue;
	    }
	for (p = line; *p != '\0'; p++)
	    if (*p == ':')
		{
		puts("Sorry, no colons allowed; they cause nasty surprises.");
		log("Illegal colon in name");
		break;
		}
	}
	while (*p != NULL);
    strncpy(nubuf.u_name, line, 32);
    nubuf.u_name[32] = '\0';
    line[0] = '\0';
    do
	{
	if (line[0] != 0)
	    puts("You made a typing error.");
	strcpy(line, getpass("Please enter a password of three to eight characters.\nIt will not be displayed: "));
	log("Pass: %s", line);
	}
	while (strlen(line) < 3 || strcmp(line, getpass("Please re-enter it, just to make sure: ")) != 0);
    strcpy(nubuf.u_pass, line);
    do
	{
	printf("How many characters per line are on your terminal?\nPlease enter a number from 40 to 132, or <ENTER> for 80: ");
	gets(line);
	log("Line: %s", line);
	if (line[0] == '\0')
	    nubuf.u_llen = 80;
	else
	    nubuf.u_llen = atoi(line);
	}
	while (nubuf.u_llen < 40 || nubuf.u_llen > 132);
    printf("\nName:\t%s\nPass:\t%s\nLine:\t%d\n\nIs this correct (N)? ", nubuf.u_name, nubuf.u_pass, nubuf.u_llen);
    gets(line);
    log("Okay? %s", line);
    if (ToLower(line[0]) != 'y')
	goto Again;
    puts("Encrypting password, please wait...");
    strcpy(nubuf.u_pass, crypt(nubuf.u_pass, nubuf.u_pass) + 2);
    strcpy(nubuf.u_login, user.u_login);/* default login name ( guest ?) */
    nubuf.u_access = user.u_llen;	/* since we don't use u_llen here */
    nubuf.u_nbull = 0;			/* no bulletins read yet */
    puts("Recording user information...");
    for (p = nubuf.u_name; *p != '\0'; p++)
	*p = ToLower(*p);
    if (getuser(nubuf.u_name, &junk))
	{
	puts("Sorry, but that name's already in use.  Please choose another.");
	goto Again;
	}
    putuser(nubuf.u_name, &nubuf);
    user = nubuf;
    }

userctl(s)
    char *s;
    {
    char line[256], *p, *q;
    struct user ubuf;
    short cflag, pflag;

    if (user.u_access != A_WITNESS)
	{
	if (strcmp(user.u_name, "guest") == 0) {
	    log("Security violation:  userctl by GUEST");
	    puts("Sorry, but GUEST can't change himself.");
	    return 1;
	}
	pflag = 1;
	log("Userctl by non-Witness; restricting control modes.");
	puts("Since you're not a Fairwitness, you can only change some things about\nyourself, like your password.");
	strcpy(line, user.u_name);
	}
    else
	{
	line[0] = '\0';
	pflag = 0;
	for (p = s; *p != '\0'; p++)
	    if (*p == ' ')
		{
		strcpy(line, ++p);
		break;
		}
	if (line[0] == '\0')
	    {
	    printf("Examine which user: ");
	    gets(line);
	    log("User: %s", line);
	    if (line[0] == '\0')
		return 1;
	    for (p = line; *p != '\0'; p++)
		*p = ToLower(*p);
	    }
	line[32] = '\0';
	}
    if (!getuser(line, &ubuf))
	if (pflag)
	    {
	    log("Can't locate current user in the userfile.");
	    panic("user");
	    }
	else
	    {
	    printf("No such user.  Create him (N)? ");
	    strcpy(ubuf.u_name, line);
	    gets(line);
	    log("New user? %s", line);
	    if (ToLower(line[0]) != 'y')
		return 1;
	    ubuf.u_pass[0] = '\0';
	    ubuf.u_access = A_USER;
	    ubuf.u_llen = 80;
	    ubuf.u_nbull = 0;
	    cflag = 0;
	    }
    else if (strlen(ubuf.u_pass) == 0)
	cflag = 0;
    else
	cflag = 1;
    for (;;)
	{
	printf("\nName:\t%s\nPass:\t%s%s\nAccess:\t%s\n%s:\t%d\nLogin conference: %s\n\nChange Name, Pass, Access, Login, %s,\nDefault Login Conference; Quit; or Save: ", ubuf.u_name, ubuf.u_pass,
	    (cflag? " (encrypted)": ""), (ubuf.u_access==A_NONE? "None": (ubuf.u_access==A_GUEST? "Guest": (ubuf.u_access==A_USER? "Ordinary user": (ubuf.u_access==A_SYSTEM? "System": (ubuf.u_access==A_FILES? "Files":
	    (ubuf.u_access==A_WITNESS? "Fairwitness": "User maker")))))), (ubuf.u_access==A_MKUSER? "DftAxs": "Width"), ubuf.u_llen, ubuf.u_lconf, (ubuf.u_access==A_MKUSER? "Default Access": "Width"));
	gets(line);
	log("Change: %s", line);
	switch (line[0])
	    {
	    case 'N':
	    case 'n':
		if (pflag)
		    {
		    log("Security violation: Attempted to change name.");
		    puts("You can't do that.");
		    break;
		    }
		printf("Enter new name: ");
		gets(line);
		log("Name: %s", line);
		if (line[0] == '\0')
		    break;
		for (p = line; *p != '\0'; p++)
		    if (*p == ':')
			{
			log("Illegal colon in name.");
			puts("Can't put a colon in a user name.");
			break;
			}
		for (p = line, q = ubuf.u_name; *p != '\0'; p++, q++)
		    *q = ToLower(*p);
		*q = '\0';
		break;
	    case 'P':
	    case 'p':
		strcpy(line, getpass("Enter new password: "));
		if (line[0] == '\0')
		    break;
		strcpy(ubuf.u_pass, line);
		cflag = 0;		/* it's not encrypted now */
		break;
	    case 'A':
	    case 'a':
		if (pflag)
		    {
		    log("Security violation: Attempted to change access level.");
		    puts("You can't do that.");
		    break;
		    }
		printf("Access: None, Guest, User, Files, System, Witness, Makeuser? ");
		gets(line);
		log("Access: %s", line);
		if ((ToLower(line[0]) == 'a' || ubuf.u_access == A_WITNESS) && strcmp(user.u_name, SYSOP) != 0)
		    {
		    puts("Sorry, only the sysop can administer Witness privileges.");
		    log("Security violation: WITNESS administering WITNESS");
		    break;
		    }
		switch (line[0])
		    {
		    case 'g':
		    case 'G':
			ubuf.u_access = A_GUEST;
			break;
		    case 'n':
		    case 'N':
			ubuf.u_access = A_NONE;
			break;
		    case '\0':
			break;
		    case 'u':
		    case 'U':
			ubuf.u_access = A_USER;
			break;
		    case 's':
		    case 'S':
			ubuf.u_access = A_SYSTEM;
			break;
		    case 'w':
		    case 'W':
			ubuf.u_access = A_WITNESS;
			break;
		    case 'm':
		    case 'M':
			ubuf.u_access = A_MKUSER;
			break;
		    case 'f':
		    case 'F':
		        ubuf.u_access = A_FILES;
		        break;
		    default:
			puts("What?  Access unchanged.");
		    }
		break;
            case 'D':
            case 'd':
                printf("Enter the default login conference: ");
                gets(line);
                log("Login conference: %s", line);
                if (!isconf(line))
                    puts("That conference doesn't exist.");
                else if (uisunsub(ubuf.u_name, line))
                    printf("%s isn't subscribed to %s.\n", ubuf.u_name, line);
                else
                    strcpy(ubuf.u_lconf, line);
                break;
	    case 'W':
	    case 'w':
		if (ubuf.u_access == A_MKUSER) {
		    printf("Default Access: None, Guest, User, Files, System? ");
		    gets(line);
		    log("DftAxs: %s", line);
		    if (ToLower(line[0]) == 'a') {
		        puts("I don't think you really want to make every user a Fairwitness.");
		        log("Security violation: DftAxs == A_WITNESS?");
		        break;
		    }
		    switch (line[0]) {
		        case 'g':
		        case 'G':
			    ubuf.u_llen = A_GUEST;
			    break;
		        case 'n':
		        case 'N':
			    ubuf.u_llen = A_NONE;
			    break;
		        case '\0':
			    break;
		        case 'u':
		        case 'U':
			    ubuf.u_llen = A_USER;
			    break;
		        case 's':
		        case 'S':
			    ubuf.u_access = A_SYSTEM;
			    break;
		        case 'm':
		        case 'M':
			    puts("Default access is user maker???");
			    log("Attempted to make default access == MAKEUSER?");
			    break;
		        case 'f':
		        case 'F':
		            ubuf.u_access = A_FILES;
		            break;
		        default:
			    puts("What?  Default access unchanged.");
		    }
		}
		else {
		    printf("Enter new line length, 40-132: ");
		    gets(line);
		    log("Line length: %s", line);
		    if (line[0] == '\0')
		        break;
		    ubuf.u_llen = atoi(line);
		}
		break;
	    case 'Q':
	    case 'q':
		printf("Abort user examine, are you sure (N)? ");
		gets(line);
		log("Abort? %s", line);
		if (ToLower(line[0]) != 'y')
		    break;
		return 1;
	    case 'S':
	    case 's':
		if (!cflag)
		    {
		    puts("Encrypting password, please wait...");
		    strcpy(ubuf.u_pass, crypt(ubuf.u_pass, ubuf.u_pass) + 2);
		    }
		putuser(ubuf.u_name, &ubuf);
		if (strcmp(ubuf.u_name, user.u_name) == 0)
		    user = ubuf;
		return 1;
	    default:
		puts("What?  Please enter one of N, P, L, A, D, or S.");
	    }
	}
    }

userlist()
    {
    FILE *bfd;
    char line[1024], *p, *q, dbuf[20];
    short lcnt, ncolon;
    struct user buf;

    if ((bfd = fopen(PASSWD, "r")) == NULL)
	{
	log("Error %d opening %s", errno, PASSWD);
	panic("passwd");
	}
    puts("\nList of UNaXcess users:\n");
    lcnt = 0;
    while (fgets(line, 1024, bfd) != NULL)
	{
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
	if (ncolon < 6) {
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
	for (p++, q = buf.u_lconf; *p != '\n'; p++, q++)
		*q = *p;
	*q = '\0';
	for (p = buf.u_name; *p != NULL; p++)
	    *p = ToUpper(*p);
	printf("%-32.32s Access: %s\n", buf.u_name, (buf.u_access==A_NONE?
	    "None": (buf.u_access==A_GUEST? "Guest": (buf.u_access==A_USER?
	    "Normal": (buf.u_access==A_WITNESS? "Fairwitness": (buf.u_access==A_SYSTEM? "System": (user.u_access==A_FILES? "Files": "(make a user)")))))));
	if (++lcnt % 16 == 0)
	    if (!cont())
		break;
	}
    fclose(bfd);
    return 1;
    }
