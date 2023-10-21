/*
 *	@(#)conf.c	1.1 (TDI) 7/18/86 21:01:41
 *
 * Modification History:
 *
 *   Ver. 1.1 created 07/18/86 at 21:01:41 by brandon
 *     Converted to SCCS 07/18/86 21:01:40
 *	@(#)Copyright (C) 1984, 85, 86 by Brandon S. Allbery.
 *
 * Modification History:
 *
 *   Ver. 1.1 created 07/18/86 at 21:01:41 by brandon
 *     Converted to SCCS 07/18/86 21:01:40
 *
 *	@(#)This file is part of UNaXcess version 0.4.5.
 *
 * Modification History:
 *
 *   Ver. 1.1 created 07/18/86 at 21:01:41 by brandon
 *     Converted to SCCS 07/18/86 21:01:40
 *
 *	Permission is hereby granted to copy and distribute this program
 *	freely.  Permission is NOT given to modify this program or distribute
 *	it at cost, except for charging a reasonable media/copying fee.
 */

#ifndef lint
static char _FileID_[] = "@(#)conf.c	1.1 (TDI) 7/18/86 21:01:41";
static char _UAID_[]   = "@(#)UNaXcess version 0.4.5";
#endif lint

#include "ua.h"

char conference[33];

confidx()
    {
    FILE *ifd;
    short icnt, himsg;
    char line[256];
    DIR *dp;
    struct direct *dfd;

    icnt = 0;
    puts("\n\"!\" after a conference name indicates an unsubscribed-to conference.\n\nConference                        HiMsg Conference                        HiMsg");
    if ((dp = opendir(MSGBASE)) == NULL)
	{
	log("Error %d opening dir %s/", errno, MSGBASE);
	panic("msgdir");
	}
    while ((dfd = readdir(dp)) != NULL)
	{
	if (dfd->d_name[0] == '.')
	    continue;
	sprintf(line, "%s/%s/himsg", MSGBASE, dfd->d_name);
	if ((ifd = fopen(line, "r")) == NULL)
	    {
	    log("No himsg in conference %s", dfd->d_name);
	    continue;
	    }
	fgets(line, 32, ifd);
	himsg = atoi(line);
	printf("%-32.32s%c %-5d", dfd->d_name, (isunsub(dfd->d_name)? '!': ':'), himsg);
	fclose(ifd);
	if (user.u_llen < 80 || ++icnt % 2 == 0)
	    putchar('\n');
	else if (icnt % 2 != 0)
	    putchar('|');
	}
    closedir(dp);
    puts("\n");
    return 1;
    }

join(c)
    char *c;
    {
    char line[256], *p;

    p = c - 1;
    while (*++p != '\0')
	if (*p == ' ')
	    if (verify(++p))
		{
		strcpy(conference, p);
		return 1;
		}
    do
	{
	printf("Enter conference: ");
	gets(line);
	log("Enter conference: %s", line);
	if (line[0] == '\0')
	    return 1;
	}
    while (!verify(line));
    strcpy(conference, line);
    log("Current conference is %s", conference);
    return 1;
    }

verify(conf)
    char *conf;
    {
    char *cp, line[256];

    for (cp = conf; *cp != 0; cp++)
	{
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
	{
	if (errno != ENOENT)
	    {
	    log("Error %d accessing dir %s/", errno, line);
	    return 0;
	    }
	else
	    return newconf(conf);
	}
    chdir("../..");
    if (isunsub(conf)) {
        printf("You are unsubscribed from this conference.  Rejoin (N)? ");
        gets(line);
        log("Unsubscribed.  Resubscribe? %s", line);
        if (ToLower(line[0]) == 'y')
            resubscribe(conf);
        else
            return 0;
    }
    if (parms.ua_xrc && conf[0] == 'x' && conf[1] == '-')
	{
	printf("This is a Restricted Access (X-RATED) conference.  The material within\n may be unsuitable for, or unacceptable to, some users of UNaXcess.\n\nDo you still wish to join this conference (N)? ");
	gets(line);
	log("Restricted.  Join? %s", line);
	return (ToLower(line[0]) == 'y');
	}
    return 1;
    }

killmsg(n)
    char *n;
    {
    short mnum, flag;
    char line[256], *p;

    if (user.u_access == A_GUEST)
	{
	puts("You aren't authorized for this function.");
	log("Security violation:  KILL by a GUEST");
	return 1;
	}
    flag = 0;
    for (p = n; *p != '\0'; p++)
	if (*p == ' ')
	    {
	    if ((mnum = atoi(++p)) < 1)
		break;
	    dokill(mnum);
	    flag++;
	    }
    if (flag)
	return 1;
    printf("Enter message number to kill: ");
    gets(line);
    if (line[0] == '\0')
	return 1;
    if ((mnum = atoi(line)) < 1)
	{
	puts("Bad message number.");
	log("Bad message number: %s", line);
	return 1;
	}
    dokill(mnum);
    return 1;
    }

dokill(msg)
    short msg;
    {
    char mfile[256];

    sprintf(mfile, "%s/%s/%d", MSGBASE, conference, msg);
    if (user.u_access != A_WITNESS && strcmp(getowner(mfile), user.u_name) != 0)
	{
	puts("Sorry, you don't own that message.");
	log("Security violation:  KILL by non-owner");
	return;
	}
    if (unlink(mfile) < 0)
	{
	printf("No such message: %d", msg);
	log("Error %d unlinking %s", errno, mfile);
	return;
	}
    log("Deleted %s:%d", conference, msg);
    }

char *getowner(file)
    char *file;
    {
    FILE *f;
    char line[1024], *p;
    static char owner[256];

    strcpy(owner, parms.ua_sysop);
    if ((f = fopen(file, "r")) == NULL)
	return owner;
    while (fgets(line, 1024, f) != NULL)
	if (line[0] == '\n')
	    break;
	else if (strncmp(line, "From: ", 6) == 0)
	    {
	    strcpy(owner, &line[6]);
	    break;
	    }
    fclose(f);
    for (p = owner; *p != '\0'; p++)
	*p = ToLower(*p);
    return owner;
    }

newconf(conf)
    char *conf;
    {
    char line[256];
    FILE *f;

    if (user.u_access == A_GUEST)
	{
	log("Security violation:  attempted MKCONF by guest");
	puts("Sorry, there is no such conference.");
	return 0;
	}
    printf("There is no conference by that name.  Do you want to create it (N)? ");
    gets(line);
    log("Nonexistent.  Create? %s", line);
    if (ToLower(line[0]) != 'y')
	return 0;
    if (parms.ua_xrc && conf[0] == 'x' && conf[1] == '-')
	{
	printf("Conferences beginning with \"x-\" are designated as Restricted Access (X-RATED)\nconferences under UNaXcess, and will often carry information unsuitable for some\n");
	printf("users or unacceptable to some users.  If you do not wish to create such a\nconference, answer NO to the next question and choose a conference name not\n");
	printf("beginning with the characters \"x-\".\n\nDo you wish to create an X-RATED conference (N)? ");
	gets(line);
	log("Restricted.  Create? %s", line);
	if (ToLower(line[0]) != 'y')
	    return 0;
	}
    if (parms.ua_roc && conf[0] == 'r' && conf[1] == '-')
	if (user.u_access != A_WITNESS)
	    {
	    puts("Only Fairwitnesses can make READ-ONLY conferences.");
	    log("Attempted mk of RO conf by non-FW");
	    return 0;
	    }
	else
	    {
	    puts("This conference will be READ-ONLY, except to Fairwitnesses.\nIf you want anyone to be able to add to it, answer NO and use a name not\nbeginning with \"R-\".");
	    printf("\nDo you want to make this READ-ONLY conference (N)? ");
	    gets(line);
	    log("Read-only.  Create? %s", line);
	    if (ToLower(line[0]) != 'y')
		return 0;
	    }
#ifdef BSD
    sprintf(line, "%s/%s", MSGBASE, conf);
    if (mkdir(line) < 0) {
    	log("Mkconf of %s failed", conf);
    	puts("Hmmm... guess you aren't allowed.");
    	return 0;
    }
    chown(line, geteuid(), getegid());
#else  !BSD
    sprintf(line, "exec mkconf %s/%s %d", MSGBASE, conf, geteuid());
    if (system(line) != 0)
	{
	log("Mkconf of %s failed.", conf);
	puts("Hmmm... guess you aren't allowed.");
	return 0;
	}
#endif BSD
    log("New conference: %s", conf);
    sprintf(line, "%s/%s/himsg", MSGBASE, conf);
    if ((f = fopen(line, "w")) == NULL)
	{
	log("Error %d opening %s", line);
	puts("Can't create high message file.  Strange...");
	return 0;
	}
    fputs("0\n", f);
    fclose(f);
    puts("You will now be placed in the message editor to make a message describing\nthis conferemnce.  It will be addressed to, and readable by, all users.");
    mkmsg("All", "This conference", conf, 0);
    return 1;
    }

isprivate(msg)
    char *msg;
    {
    FILE *fp;
    char line[1024], to[1024], from[1024];
    short pflag;

    if (user.u_access == A_WITNESS)
	return 0;
    if ((fp = fopen(msg, "r")) == NULL)
	return 0;
    strcpy(to, "All");
    pflag = 0;
    while (fgets(line, 1024, fp) != NULL)
	{
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
    if (pflag && strcmp(user.u_name, to) == 0)
	return 0;
    else if (pflag && strcmp(user.u_name, from) == 0)
	return 0;
    else if (pflag)
	{
	log("Message %s is private.", msg);
	return 1;
	}
    else
	return 0;
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
        puts("Can't unsubscribe the general conference.");
        log("Attempted to unsubscribe to general.");
        return;
    }
    if (s_cmp(conf, user.u_lconf) == 0) {
        printf("Unsubscribe to login conference (N)? ");
        gets(line);
        log("Unsub login conf? %s", line);
        if (ToLower(line[0]) != 'y')
            return;
        strcpy(user.u_lconf, "general");
    }
    for (hip = hicnts; hip != NULL; hip = hip->hi_next)
        if (strcmp(hip->hi_conf, conf) == 0)
            break;
    if (hip != NULL)
        hip->hi_uns = HI_UNSUB;
    else {
	if ((workp = (struct _himsg *) calloc((unsigned) 1, sizeof (struct _himsg))) == NULL)
	    {
	    log("Error %d allocating _himsg for %s", errno, conf);
	    panic("alloc");
	    }
	strcpy(workp->hi_conf, conf);
	workp->hi_num = 0;
	workp->hi_next = hicnts;
	hicnts = workp;
	workp->hi_uns = HI_UNSUB;
    }
    log("Unsubscribed to %s", conf);
    printf("Unsubscribed to conference %s.\n", conf);
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
	if ((workp = (struct _himsg *) calloc((unsigned) 1, sizeof (struct _himsg))) == NULL)
	    {
	    log("Error %d allocating _himsg for %s", errno, conf);
	    panic("alloc");
	    }
	strcpy(workp->hi_conf, conf);
	workp->hi_num = 0;
	workp->hi_next = hicnts;
	hicnts = workp;
	workp->hi_uns = HI_SUBSCR;
    }
    log("Resubscribed to %s", conf);
    printf("Resubscribed to conference %s.\n", conf);
}

unsub(c)
char *c; {
    char line[256], *p;

    p = c - 1;
    while (*++p != '\0')
	if (*p == ' ')
	    if (isconf(++p))
		{
		unsubscribe(p);
		return 1;
		}
    for (;;) {
	printf("Unsubscribe to which conference (%s) [NONE to abort]: ", conference);
	gets(line);
	log("Unsub conference: %s", line);
	if (line[0] == '\0') {
	    unsubscribe(conference);
	    return 1;
	}
	if (s_cmp(line, "none") == 0)
	    return 1;
	if (isconf(line)) {
	    unsubscribe(line);
	    return 1;
	}
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
    chdir("../..");
    return 1;
}

setlconf(conf)
char *conf; {
    char line[256], *p;

    if (s_cmp(user.u_name, "guest") == 0) {
        log("Guest SET LOGIN CONF denied.");
        puts("GUEST can't set a login conference.");
        return 1;
    }
    p = conf - 1;
    while (*++p != '\0')
	if (*p == ' ')
	    if (isconf(++p)) {
                if (isunsub(p)) {
                    puts("You're unsubscribed from it.  <J>oin it and resubscribe.");
                    log("Unsubscribed -- login conf set aborted.");
                    return 1;
                }
		strcpy(user.u_lconf, p);
		log("New login conference: %s", user.u_lconf);
                putuser(user.u_name, &user);
		return 1;
	    }
    do {
	printf("Enter new login conference: ");
	gets(line);
	log("Login conference: %s", line);
	if (line[0] == '\0')
	    return 1;
    } while (!isconf(line));
    if (isunsub(line)) {
        puts("You're unsubscribed from it.  <J>oin it and resubscribe.");
        log("Unsubscribed -- login conf set aborted.");
        return 1;
    }
    strcpy(user.u_lconf, line);
    log("New login conference: %s", user.u_lconf);
    putuser(user.u_name, &user);
    return 1;
}

uisunsub(user, conf)
char *user, *conf; {
    struct _himsg *hip, *uhi;
    char *cp;

    for (cp = user; *cp != '\0'; cp++)
        *cp = ToLower(*cp);
    if ((uhi = readhigh(user)) < 0) {
        log("Couldn't read %s's userindex.", user);
        return 0;
    }
    printf("Checking %s's user index...\n", user);
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
    DIR *conferences;
    struct direct *conf;
    
    lastp = NULL;
    puts("Checking for deleted conferences...");
    for (hip = hicnts; hip != NULL; lastp = hip, hip = hip->hi_next) {
        if (!isconf(hip->hi_conf)) {
            printf("Conference \"%s\" was deleted since your last session.\n", hip->hi_conf);
            if (lastp == NULL)
                hicnts = hip->hi_next;
            else
                lastp->hi_next = hip->hi_next;
            free((char *) hip);
        }
    }
    puts("\nChecking for new conferences...");
    if ((conferences = opendir(MSGBASE)) == NULL) {
        log("Error %d opening dir %s/", errno, MSGBASE);
        panic("msgdir");
    }
    while ((conf = readdir(conferences)) != NULL) {
        if (strcmp(conf->d_name, ".") == 0)
            continue;
        if (strcmp(conf->d_name, "..") == 0)
            continue;
        for (hip = hicnts; hip != NULL; hip = hip->hi_next)
            if (strcmp(hip->hi_conf, conf->d_name) == 0)
                break;
        if (hip == NULL)
            printf("Conference \"%s\" has been created since your last session.\n", conf->d_name);
    }
    closedir(conferences);
}
