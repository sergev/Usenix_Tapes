/*
 *	@(#)date.c	1.1 (TDI) 7/18/86 21:01:43
 *
 * Modification History:
 *
 *   Ver. 1.1 created 07/18/86 at 21:01:43 by brandon
 *     Converted to SCCS 07/18/86 21:01:43
 *	@(#)Copyright (C) 1984, 85, 86 by Brandon S. Allbery.
 *
 * Modification History:
 *
 *   Ver. 1.1 created 07/18/86 at 21:01:43 by brandon
 *     Converted to SCCS 07/18/86 21:01:43
 *
 *	@(#)This file is part of UNaXcess version 0.4.5.
 *
 * Modification History:
 *
 *   Ver. 1.1 created 07/18/86 at 21:01:43 by brandon
 *     Converted to SCCS 07/18/86 21:01:43
 *
 *	Permission is hereby granted to copy and distribute this program
 *	freely.  Permission is NOT given to modify this program or distribute
 *	it at cost, except for charging a reasonable media/copying fee.
 */

#ifndef lint
static char _FileID_[] = "@(#)date.c	1.1 (TDI) 7/18/86 21:01:43";
static char _UAID_[]   = "@(#)UNaXcess version 0.4.5";
#endif lint

#ifdef BSD
#include <sys/time.h>
#else
#include <time.h>
#endif BSD

static char *month[] =
    {
	"January",	"February",	"March",	"April",
	"May",		"June",		"July",		"August",
	"September",	"October",	"November",	"December"
    };

static char *wkday[] =
    {
	"Sunday",	"Monday",	"Tuesday",	"Wednesday",
	"Thursday",	"Friday",	"Saturday"
    };

struct tm *localtime();

char *date()
    {
    long clock;
    struct tm *ltbuf;
    static char tbuf[18];

    time(&clock);
    ltbuf = localtime(&clock);
    sprintf(tbuf, "%02d/%02d/%02d %02d:%02d:%02d", ltbuf->tm_mon + 1, ltbuf->tm_mday, ltbuf->tm_year, ltbuf->tm_hour, ltbuf->tm_min, ltbuf->tm_sec);
    return tbuf;
    }

char *longdate()
    {
    long clock;
    struct tm *ltbuf;
    static char tbuf[80];
    short hour;
    char ampm;

    time(&clock);
    ltbuf = localtime(&clock);
    if (ltbuf->tm_hour == 0)
	{
	hour = 12;
	ampm = 'A';
	}
    else if (ltbuf->tm_hour < 12)
	{
	hour = ltbuf->tm_hour;
	ampm = 'A';
	}
    else if (ltbuf->tm_hour == 12)
	{
	hour = 12;
	ampm = 'P';
	}
    else
	{
	hour = ltbuf->tm_hour - 12;
	ampm = 'P';
	}
    sprintf(tbuf, "%s, %s %d, 19%02d - %d:%02d %cM", wkday[ltbuf->tm_wday], month[ltbuf->tm_mon], ltbuf->tm_mday, ltbuf->tm_year, hour, ltbuf->tm_min, ampm);
    return tbuf;
    }
