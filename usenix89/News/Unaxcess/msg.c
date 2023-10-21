/*
 *	@(#)msg.c	1.1 (TDI) 7/18/86 21:01:49
 *
 * Modification History:
 *
 *   Ver. 1.1 created 07/18/86 at 21:01:49 by brandon
 *     Converted to SCCS 07/18/86 21:01:48
 *	@(#)Copyright (C) 1984, 85, 86 by Brandon S. Allbery.
 *
 * Modification History:
 *
 *   Ver. 1.1 created 07/18/86 at 21:01:49 by brandon
 *     Converted to SCCS 07/18/86 21:01:48
 *
 *	@(#)This file is part of UNaXcess version 0.4.5.
 *
 * Modification History:
 *
 *   Ver. 1.1 created 07/18/86 at 21:01:49 by brandon
 *     Converted to SCCS 07/18/86 21:01:48
 *
 *	Permission is hereby granted to copy and distribute this program
 *	freely.  Permission is NOT given to modify this program or distribute
 *	it at cost, except for charging a reasonable media/copying fee.
 */

#ifndef lint
static char _FileID_[] = "@(#)msg.c	1.1 (TDI) 7/18/86 21:01:49";
static char _UAID_[]   = "@(#)UNaXcess version 0.4.5";
#endif lint

#include "ua.h"

selmsg(s, fn)
    char *s;
    int (*fn)();
    {
    char line[256], *p;
    short lomsg, himsg;
    FILE *f;

    sprintf(line, "%s/%s/himsg", MSGBASE, conference);
    if ((f = fopen(line, "r")) == NULL)
	{
	log("Error %d opening %s", errno, line);
	if (strcmp(conference, "general") == 0)
	    {
	    panic("conf");
	    }
	puts("I can't find the high message file.  Moving back to general...");
	strcpy(conference, "general");
	return 1;
	}
    fgets(line, 32, f);
    fclose(f);
    himsg = atoi(line);
    for (p = s; *p != 0; p++)
	if (*p == ' ')
	    {
	    if (strcmp(++p, "new") == 0)
		{
		domsg(conference, 0, himsg, fn);
		return 1;
		}
	    else if ((lomsg = atoi(p)) < 1 || lomsg > himsg)
		break;
	    else
		{
		domsg(conference, lomsg, lomsg, fn);
		return 1;
		}
	    }
    printf("<F>orward, <R>everse, <I>ndividual, or <N>ew: ");
    gets(line);
    log("Mode: %s", line);
    switch (line[0])
	{
	case 'F':
	case 'f':
	    lomsg = 1;
	    break;
	case 'R':
	case 'r':
	    lomsg = himsg;
	    himsg = 1;
	    break;
	case 'I':
	case 'i':
	    printf("Enter message number: ");
	    gets(line);
	    log("Message: %s", line);
	    if ((lomsg = atoi(line)) < 1 || lomsg > himsg)
		{
		puts("No such message.");
		log("No such message.");
		return 1;
		}
	    domsg(conference, lomsg, lomsg, fn);
	    return 1;
	case 'N':
	case 'n':
	    lomsg = 0;
	    break;
	case '\0':
	    return 1;
	default:
	    puts("What?");
	    log("Illegal search mode.");
	    return 1;
	}
    if (lomsg != 0)
	{
	printf("Starting message (%d): ", lomsg);
	gets(line);
	log("Start: %s", line);
	if (line[0] != 0)
	    if (atoi(line) < 1 || (lomsg > 1 && atoi(line) > lomsg))
		{
		puts("Bad message number.");
		log("Bad message number.");
		return 1;
		}
	    else
		lomsg = atoi(line);
	printf("Ending message (%d): ", himsg);
	gets(line);
	log("End: %s", line);
	if (line[0] != 0)
	    if (atoi(line) < 1 || (himsg > 1 && atoi(line) > himsg))
		{
		puts("Bad message number.");
		log("Bad message number.");
		return 1;
		}
	    else
		himsg = atoi(line);
	}
    domsg(conference, lomsg, himsg, fn);
    return 1;
    }

readmsg(s)
    char *s;
    {
    return selmsg(s, doread);
    }

scanmsg(s)
    char *s;
    {
    return selmsg(s, doscan);
    }

doread(msg, conf, mnum)
    char *msg, *conf;
    short mnum;
    {
    char line[256];

    printf("\nMessage %d of %s:\n", mnum, conf);
    if (isprivate(msg))
	{
	puts("This message is private.");
	return 1;
	}
    cat(msg);

DR_Loop:
    printf("\nContinue, Stop, Unsubscribe, or Reply (C): ");
    if (!isatty(0) || nopause)
	{
	line[0] = '\0';
	putchar('\n');
	}
    else
	gets(line);
    log("C/S/U/R: %s", line);
    switch (line[0])
	{
	case 'c':
	case 'C':
	case '\0':
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
	default:
	    puts("What?  Please enter one of C, S, U, or R.");
	    goto DR_Loop;
	}
    }

msgok(file)
    char *file;
    {
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
	puts("Cannot open file.");
	log("Error %d opening %s", errno, msg);
	return 1;
    }
    printf("\nMessage %d of %s: \n", mnum, conf);
    dflag = fflag = tflag = sflag = 0;
    if (isprivate(msg))
	puts("Message is private.");
    else {
	while (fgets(line, 1024, f) != NULL) {
	    if (line[0] == '\n')
		break;
	    if (!dflag && strncmp(line, "Date: ", 6) == 0) {
		printf("%s", line);
		dflag++;
		continue;
	    }
	    if (!fflag && strncmp(line, "From: ", 6) == 0) {
		printf("%s", line);
		fflag++;
		continue;
	    }
	    if (!tflag && strncmp(line, "To: ", 4) == 0) {
		printf("%s", line);
		tflag++;
		continue;
	    }
	    if (!sflag && strncmp(line, "Subject: ", 9) == 0) {
		printf("%s", line);
		sflag++;
		continue;
	    }
	    if (!sflag && strncmp(line, "Subject (Private): ", 19) == 0) {
		printf("%s", line);
		sflag++;
		continue;
	    }
	}
        if (!tflag)
	    puts("To: All");
    }
    fclose(f);
    puts("--------------------------------");
    if (mnum % 3 == 0)	/* kludged, but there isn't an easy fix without */
        if (!cont())	/* rewriting the I/O system. */
            longjmp(cmdloop, 1);	/* also a kludge... */
    return 1;
    }

domsg(conf, lomsg, himsg, fn)
    char *conf;
    short lomsg, himsg;
    int (*fn)();
    {
    short mcnt;
    char tmps[256];
    struct _himsg *ptr, *lastp;

    for (ptr = hicnts, lastp = NULL; ptr != NULL; lastp = ptr, ptr = ptr->hi_next)
	if (strcmp(conf, ptr->hi_conf) == 0)
	    break;
    if (ptr == NULL)
	{
	if ((ptr = (struct _himsg *) calloc((unsigned) 1, sizeof (struct _himsg))) == NULL)
	    {
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
	for (mcnt = ptr->hi_num + 1; mcnt <= himsg; mcnt++)
	    {
	    sprintf(tmps, "%s/%s/%d", MSGBASE, conf, mcnt);
	    if (msgok(tmps) <= 0)
		continue;
	    if (!(*fn)(tmps, conf, mcnt))
		break;
	    }
    else if (lomsg <= himsg)		/* forward or individual read */
	for (mcnt = lomsg; mcnt <= himsg; mcnt++)
	    {
	    sprintf(tmps, "%s/%s/%d", MSGBASE, conf, mcnt);
	    if (msgok(tmps) <= 0)
		continue;
	    if (!(*fn)(tmps, conf, mcnt))
		break;
	    }
    else
	for (mcnt = lomsg; mcnt >= himsg; mcnt--)
	    {
	    sprintf(tmps, "%s/%s/%d", MSGBASE, conf, mcnt);
	    if (msgok(tmps) <= 0)
		continue;
	    if (!(*fn)(tmps, conf, mcnt))
		break;
	    }
    ptr->hi_num = himsg;
    writehigh(hicnts);
    }

readnew()
    {
    DIR *dp;
    struct direct *dirp;
    FILE *hp;
    short himsg;
    char line[256];

    if ((dp = opendir(MSGBASE)) == NULL)
	{
	log("Error %d reading dir %s/", errno, MSGBASE);
	panic("msgdir");
	}
    while ((dirp = readdir(dp)) != NULL)
	{
	if (dirp->d_name[0] == '.')
	    continue;
        if (isunsub(dirp->d_name))
            continue;
	printf("\nExamining conference %s...\n", dirp->d_name);
	log("Reading %s.", dirp->d_name);
	if (parms.ua_xrc && dirp->d_name[0] == 'x' && dirp->d_name[1] == '-')
	    {
            if (user.u_access == A_GUEST) {
                log("Guest skipping Restricted conference.");
                continue;
            }
	    printf("This conference is Restricted (X-RATED).  The material within may not be\nsuitable for, or acceptable to, some users.\n\nDo you wish to skip it (Y)? ");
	    if (!isatty(0) || nopause)
		{
		line[0] = '\0';
		putchar('\n');
		}
	    else
		gets(line);
	    log("Restricted.  Skip? %s", line);
	    if (ToLower(line[0]) != 'n')
		continue;
	    }
	sprintf(line, "%s/%s/himsg", MSGBASE, dirp->d_name);
	if ((hp = fopen(line, "r")) == NULL)
	    {
	    log("Error %d opening %s", errno, line);
	    puts("Can't open high message file.");
	    continue;
	    }
	fgets(line, 32, hp);
	fclose(hp);
	himsg = atoi(line);
	domsg(dirp->d_name, 0, himsg, doread);
    
    RN_Loop:
	printf("\nNext conference, Unsubscribe, or Stop (N): ");
	if (!isatty(0) || nopause)
	    {
	    putchar('\n');
	    line[0] = '\0';
	    }
	else
	    gets(line);
	log("Next/Unsub/Stop: %s", line);
	switch (line[0])
	    {
	    case 'N':
	    case 'n':
	    case '\0':
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
		puts("Please enter one of N, U, or S.");
		goto RN_Loop;
	    }
	}
    closedir(dp);
    return 1;
    }

enter(s)
    char *s;
    {
    char to[256], subj[256], *p, line[256];
    short pflag;

    if (user.u_access == A_GUEST && strcmp(conference, "guest") != 0)
	{
	log("Security violation:  GUEST entering messages.");
	puts("You aren't allowed to enter messages in this conference.");
	return 1;
	}
    for (p = s; *p != '\0'; p++)
	if (*p == ' ')
	    {
	    strcpy(to, ++p);
	    break;
	    }
    if (*p == '\0')
	{
	printf("Who is this message to (ALL)? ");
	gets(line);
	log("To: %s", line);
	if (line[0] == '\0')
	    strcpy(line, "all");
	for (p = line; *p != '\0'; p++)
	    *p = ToLower(*p);
	strcpy(to, line);
	}
    printf("Subject: ");
    gets(line);
    strcpy(subj, line);
    log("Subject: %s", line);
    pflag = 0;
    if (parms.ua_pm) {
    	printf("Is this message to be private (N)? ");
    	gets(line);
    	log("Private? %s", line);
    	if (ToLower(line[0]) == 'y')
	    pflag = 1;
    }
    mkmsg(to, subj, conference, pflag);
    return 1;
    }

reply(msg, conf)
    char *msg, *conf;
    {
    char to[256], subj[256], line[1024], rconf[256];
    short fflag, sflag, pflag;
    FILE *f;

    if (user.u_access == A_GUEST && strcmp(conf, "guest") != 0)
	{
	log("Security violation:  GUEST entering messages");
	puts("You aren't allowed to enter messages.");
	return;
	}
    if ((f = fopen(msg, "r")) == NULL)
	{
	log("Error %d opening %s", errno, msg);
	puts("Can't re-open message file.");
	return;
	}
    fflag = sflag = 0;
    strcpy(to, "All\n");
    strcpy(subj, "Re: Orphaned Response\n");	/* now you know... */
    while (fgets(line, 1024, f) != NULL)
	{
	if (line[0] == '\n')
	    break;
	if (!fflag && strncmp(line, "From: ", 6) == 0)
	    {
	    strcpy(to, &line[6]);
	    fflag++;
	    continue;
	    }
	if (!sflag && strncmp(line, "Subject: ", 9) == 0)
	    {
	    if (strncmp(&line[9], "Re: ", 4) == 0)
		strcpy(subj, &line[9]);
	    else
		strcpy(&subj[4], &line[9]);
	    sflag++;
	    continue;
	    }
	if (!sflag && strncmp(line, "Subject (Private): ", 19) == 0)
	    {
	    if (strncmp(&line[19], "Re: ", 4) == 0)
		strcpy(subj, &line[19]);
	    else
		strcpy(&subj[4], &line[19]);
	    sflag++;
	    continue;
	    }
	}
    fclose(f);
    to[strlen(to) - 1] = '\0';			/* get rid of trailing nl */
    subj[strlen(subj) - 1] = '\0';
    printf("What conference do you wish this reply to be in (%s): ", conf);
    gets(line);
    if (line[0] != '\0' && verify(line))
	{
	strcpy(rconf, line);
	conf = rconf;
	}
    pflag = 0;
    if (parms.ua_pm) {
	    printf("Is this message to be private (N)? ");
	    gets(line);
	    log("Private? %s", line);
	    if (ToLower(line[0]) == 'y')
		pflag = 1;
    }
    mkmsg(to, subj, conf, pflag);
    }

mkmsg(to, subj, conf, ispriv)
    char *to, *subj, *conf;
    {
    static char lockfile[] = "msgbase.lock";
    char *tempfile = mktemp("/tmp/UAmXXXXXX");
    FILE *mfp, *sfp;
    char line[1024], *p;
    long clock;
    short mcnt;
    struct tm *ltbuf;
    struct user ubuf;

    if (user.u_access != A_WITNESS && parms.ua_roc && conf[0] == 'r' && conf[1] == '-')
	{
	conf = "general";			/* responses get redirected */
	puts("Read-only conference; message will be added to \"general\".");
	}
    if (ispriv && !getuser(to, &ubuf))
	{
	printf("Can't send private message to \"%s\"; he's unregistered.\n", to);
	log("Attempted private message to unregistered user.");
	return 0;
	}
    if ((mfp = fopen(tempfile, "w")) == NULL)
	{
	log("Error %d opening %s", errno, tempfile);
	panic("tmp");
	}
    for (p = to; *p != '\0'; p++)
	*p = ToUpper(*p);
    fprintf(mfp, "To: %s\nSubject%s: %s\n\n", to, (ispriv? " (Private)": ""), subj);
    fclose(mfp);
    input(tempfile);
    for (;;)
	{
	printf("\nEdit command (L, C, E, S, or A): ");
	gets(line);
	log("Edit command: %s", line);
	switch (line[0])
	    {
	    case 'l':
	    case 'L':
		cat(tempfile);
		break;
	    case 'c':
	    case 'C':
		input(tempfile);
		break;
	    case 'e':
	    case 'E':
		if (user.u_access == A_SYSTEM || user.u_access == A_WITNESS)
		    xedit(tempfile);
		else
		    edit(tempfile);
		break;
	    case 'a':
	    case 'A':
		printf("Do you really want to abort this edit (N)? ");
		gets(line);
		log("Abort? %s", line);
		if (ToLower(line[0]) == 'y')
		    {
		    unlink(tempfile);
		    return 0;
		    }
		break;
	    case '?':
		puts("Editor commands:\nL - List message\nC - Continue message entry\nE - Edit message\nS - Save message\nA - Abort message");
		break;
	    case '\0':
		break;
	    case 's':
	    case 'S':
		puts("Saving message...");
		mklock(lockfile);
		sprintf(line, "%s/%s/himsg", MSGBASE, conf);
		if ((sfp = fopen(line, "r")) == NULL)
		    {
		    log("Error %d opening %s", errno, line);
		    rmlock(lockfile);
		    unlink(tempfile);
		    panic("himsg");
		    }
		fgets(line, 32, sfp);
		fclose(sfp);
		mcnt = atoi(line) + 1;
		sprintf(line, "%s/%s/%d", MSGBASE, conf, mcnt);
		if ((sfp = fopen(line, "w")) == NULL)
		    {
		    log("Error %d opening %s", errno, line);
		    unlink(tempfile);
		    rmlock(lockfile);
		    panic("msg");
		    }
		fprintf(sfp, "Date: %s\nFrom: ", longdate());
		for (p = user.u_name; *p != '\0'; p++)
		    putc(ToUpper(*p), sfp);
		putc('\n', sfp);
		if ((mfp = fopen(tempfile, "r")) == NULL)
		    {
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
		sprintf(line, "%s/%s/himsg", MSGBASE, conf);
		if ((sfp = fopen(line, "w")) == NULL)
		    {
		    log("Error %d opening %s", errno, line);
		    panic("himsg");
		    }
		fprintf(sfp, "%d\n", mcnt);
		fclose(sfp);
		rmlock(lockfile);
		return 1;
	    default:
		puts("Please enter L, C, E, S, or A; or ? for help.");
	    }
	}
    }

input(file)
    char *file;
    {
    FILE *fp;
    char line[256], *p;

    if ((fp = fopen(file, "a")) == NULL)
	{
	log("Error %d opening %s", errno, file);
	unlink(file);
	panic("tmp");
	}
    puts("\nEnter your text now.  End it with a slash on a line by itself.\n");
    log("Entering text...");
    for (;;)
	{
	printf("] ");
	if (gets(line) == NULL)
	    {
	    log("Illegal character: EOF");
	    clearerr(stdin);			/* 4.2 brain damage fix */
	    continue;
	    }
	if (strcmp(line, "/") == 0)
	    break;
	for (p = line; *p != '\0'; p++)
	    if (iscntrl(*p) && *p != '\t')
		{
		log("Illegal character: ^%c", uncntrl(*p));
		putc('^', fp);
		putc(uncntrl(*p), fp);
		}
	    else
		putc(*p, fp);
	putc('\n', fp);
	}
    fclose(fp);
    }

edit(file)
    char *file;
    {
    char line[256], rline[256], *edtemp = mktemp("/tmp/UaEdXXXXXX");
    short lcnt, lnum;
    FILE *ip, *fp;

    for (;;)
	{
	printf("\nLine number to edit (<ENTER> to exit): ");
	gets(line);
	log("Line #: %s", line);
	if (line[0] == '\0')
	    return;
	lnum = atoi(line);
	if (lnum < 1)
	    continue;
	if ((fp = fopen(file, "r")) == NULL)
	    {
	    log("Error %d opening %s", errno, file);
	    panic("tmp");
	    }
	if ((ip = fopen(edtemp, "w")) == NULL)
	    {
	    log("Error %d opening %s", errno, edtemp);
	    puts("Can't open the temporary file.");
	    fclose(fp);
	    return;
	    }
	for (lcnt = 1; lcnt < lnum; lcnt++)
	    {
	    fgets(line, 256, fp);
	    fputs(line, ip);
	    }
	fgets(line, 256, fp);
	if (feof(fp))
	    {
	    puts("Not that many lines in the message.");
	    fclose(fp);
	    fclose(ip);
	    unlink(edtemp);
	    continue;
	    }
	printf("\nLine %d currently reads:\n\n> %s\nRe-enter the line, or hit <ENTER> to leave it unchanged:\n\n] ", lnum, line);
	gets(rline);
	log("Replacement: %s", rline);
	if (rline[0] == '\0')
	    fputs(line, ip);
	else
	    fprintf(ip, "%s\n", rline);
	while (fgets(line, 256, fp) != NULL)
	    fputs(line, ip);
	fclose(ip);
	fclose(fp);
	unlink(file);
	if (copylink(edtemp, file) < 0)
	    {
	    log("Error %d copylinking %s to %s", errno, edtemp, file);
	    panic("copylink");
	    }
	unlink(edtemp);
	}
    }
    
doqscan(msg, conf, mnum)
    char *msg, *conf;
    short mnum;
    {
    char line[1024];
    FILE *f;

    if ((f = fopen(msg, "r")) == NULL)
	{
	puts("Cannot open file.");
	log("Error %d opening %s", errno, msg);
	return 1;
	}
    printf("%5d. ", mnum);
    if (isprivate(msg))
	puts("Private message.");
    else
	while (fgets(line, 1024, f) != NULL)
	    {
	    if (line[0] == '\n')
		break;
	    if (strncmp(line, "Subject: ", 9) == 0)
		{
		printf("%s", &line[9]);
		break;
		}
	    if (strncmp(line, "Subject (Private): ", 19) == 0)
		{
		printf("%s", &line[8]);		/* include privacy tag */
		break;
		}
	    }
    fclose(f);
    if (mnum % 16 == 0)	/* kludge, see comment in doscan() */
    	if (!cont())
    	    longjmp(cmdloop, 1);
    return 1;
    }

qscan(s)
    char *s;
    {
    return selmsg(s, doqscan);
    }
