/*
 *	@(#)bull.c	1.1 (TDI) 7/18/86 21:01:39
 *
 * Modification History:
 *
 *   Ver. 1.1 created 07/18/86 at 21:01:39 by brandon
 *     Converted to SCCS 07/18/86 21:01:38
 *	@(#)Copyright (C) 1984, 85, 86 by Brandon S. Allbery.
 *
 * Modification History:
 *
 *   Ver. 1.1 created 07/18/86 at 21:01:39 by brandon
 *     Converted to SCCS 07/18/86 21:01:38
 *
 *	@(#)This file is part of UNaXcess version 0.4.5.
 *
 * Modification History:
 *
 *   Ver. 1.1 created 07/18/86 at 21:01:39 by brandon
 *     Converted to SCCS 07/18/86 21:01:38
 *
 *	Permission is hereby granted to copy and distribute this program
 *	freely.  Permission is NOT given to modify this program or distribute
 *	it at cost, except for charging a reasonable media/copying fee.
 */

#ifndef lint
static char _FileID_[] = "@(#)bull.c	1.1 (TDI) 7/18/86 21:01:39";
static char _UAID_[]   = "@(#)UNaXcess version 0.4.5";
#endif lint

#include "ua.h"

bulletin(s)
    char *s;
    {
    short mcnt, himotd;
    char tmps[256];
    FILE *fp;

    if (user.u_access == A_MKUSER)
    	return;
    sprintf(tmps, "%s/himotd", MOTD);
    if ((fp = fopen(tmps, "r")) == NULL)
	{
	log("Error %d opening %s", errno, tmps);
	panic("himotd");
	}
    fgets(tmps, 32, fp);
    fclose(fp);
    himotd = atoi(tmps);
    for (mcnt = (strcmp(user.u_name, "guest") == 0? 0: user.u_nbull + 1); mcnt <= himotd; mcnt++)
	{
	sprintf(tmps, "%s/%d", MOTD, mcnt);
	if (!readmotd(tmps, mcnt))
	    break;
	}
    }

readmotd(motd, mnum)
    char *motd;
    short mnum;
    {
    char line[256];

    printf("Bulletin #%d:\n", mnum);
    cat(motd);
    printf("\nContinue or Stop (C)? ");
    if (!isatty(0) || nopause)
	{
	putchar('\n');
	line[0] = '\0';
	}
    else
	gets(line);
    log("C/S? %s", line);
    return ToLower(line[0]) != 's';
    }
