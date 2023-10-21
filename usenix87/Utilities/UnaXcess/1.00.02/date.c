/*
 *	@(#)date.c	1.1 (TDI) 2/3/87
 *	@(#)Copyright (C) 1984, 85, 86, 87 by Brandon S. Allbery.
 *	@(#)This file is part of UNaXcess version 1.0.2.
 *
 *	Permission is hereby granted to copy and distribute this program
 *	freely.  Permission is NOT given to modify this program or distribute
 *	it at cost, except for charging a reasonable media/copying fee.
 */

#ifndef lint
static char _FileID_[] = "@(#)date.c	1.1 (TDI) 2/3/87";
static char _UAID_[]   = "@(#)UNaXcess version 1.0.2";
#endif lint

#include "ua.h"

static char *month[] = {
	"January",	"February",	"March",	"April",
	"May",		"June",		"July",		"August",
	"September",	"October",	"November",	"December"
};

static char *wkday[] = {
	"Sunday",	"Monday",	"Tuesday",	"Wednesday",
	"Thursday",	"Friday",	"Saturday"
};

char *date() {
	long clock;
	struct tm *ltbuf;
	static char tbuf[18];

	time(&clock);
	ltbuf = localtime(&clock);
	sprintf(tbuf, "%02d/%02d/%02d %02d:%02d:%02d", ltbuf->tm_mon + 1, ltbuf->tm_mday, ltbuf->tm_year, ltbuf->tm_hour, ltbuf->tm_min, ltbuf->tm_sec);
	return tbuf;
}

char *longdate() {
	long clock;
	struct tm *ltbuf;
	static char tbuf[80];
	short hour;
	char ampm;

	time(&clock);
	ltbuf = localtime(&clock);
	if (ltbuf->tm_hour == 0) {
		hour = 12;
		ampm = 'A';
	}
	else if (ltbuf->tm_hour < 12) {
		hour = ltbuf->tm_hour;
		ampm = 'A';
	}
	else if (ltbuf->tm_hour == 12) {
		hour = 12;
		ampm = 'P';
	}
	else {
		hour = ltbuf->tm_hour - 12;
		ampm = 'P';
	}
	sprintf(tbuf, "%s, %s %d, 19%02d - %d:%02d %cM", wkday[ltbuf->tm_wday], month[ltbuf->tm_mon], ltbuf->tm_mday, ltbuf->tm_year, hour, ltbuf->tm_min, ampm);
	return tbuf;
}

char *today() {
	long now;
	struct tm *datebuf;
	static char buf[11];
	
	time(&now);
	datebuf = localtime(&now);
	sprintf(buf, "%d/%d/%d", datebuf->tm_mon + 1, datebuf->tm_mday, datebuf->tm_year);
	return buf;
}
