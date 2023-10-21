
/************************************************************************
 *									*
 *		   Copyright (c) 1985, Michael Umansky			*
 *		   94 Cassia Court, Hayward  CA  94544			*
 *			       (415) 886-4805				*
 *			    All Rights Reserved				*
 *									*
 *	This software and/or documentation is released into the		*
 *	public domain for personal, non-commercial use only.		*
 *	Limited rights to use, modify, and redistribute are hereby	*
 *	granted for non-commercial purposes, provided that all		*
 *	copyright notices remain intact and all changes are clearly	*
 *	documented.  The author makes no warranty of any kind with	*
 *	respect to this product and explicitly disclaims any implied	*
 *	warranties of merchantability or fitness for any particular	*
 *	purpose. (ala other software creators)				*
 *									*
 ************************************************************************/

/*******************************************************************************
*
*	SETDATE/GETDATE
*		Utility to set and get date and time from
*		an AST SixPak expansion board on an IBM PC
*		or XT or compatibles (maybe even AT) running
*		VENIX (TM Venturcom) operating system.
*
*	TO USE:
*		In my "/etc/rc" file the last line reads:
*
*			date `getdate`
*
*		which automatically sets UNIX date in utmp file
*		by getting the date and time from the AST board
*		via the GETDATE command.
*
*	NOTE:
*		Actually the real compiled file is SETDATE and
*		GETDATE is a link to SETDATE.  SETDATE looks at
*		how it was invoked to provide different functions.
*		BEWARE: SETDATE and GETDATE expect different arguments
*			(see usage() or help() routines for descriptions).
*		This routine sets and reads date and time from/to
*		AST board in the same way as the PC-DOS commands,
*		ASTCLOCK and SETCLOCK, so that both operating systems
*		can use the same clock.  The ram buckets used by the
*		clock can even be used to pass some information
*		between the two operating systems or can even be used
*		to store some information that could be used for
*		certain setups.  This is just a sugestion for you,
*		I have not and probably will not do that.
*
*	AUTHOR:
*		Michael Umansky, Daisy Systems Corporation
*		700B Middlefield Road, Mountain View  CA  94039
*		(415) 960-7166
*		uucp - ucbvax!menlo70!nsc!daisy!misha
*
*******************************************************************************/

#include	<stdio.h>

/*
	The following are definitions for port addresses
	on the AST SixPak card.  The names are self-explanatory.
*/

#define	THOUSANDTHS	(int)0x02c0
#define	HUNDREDTHS	(int)0x02c1
#define	SECONDS		(int)0x02c2
#define	MINUTES		(int)0x02c3
#define	HOURS		(int)0x02c4
#define	WEEKDAY		(int)0x02c5
#define	DAY		(int)0x02c6
#define	MONTH		(int)0x02c7
#define	RAMTHOUSANDTHS	(int)0x02c8
#define	RAMHUNDREDTHS	(int)0x02c9
#define	RAMSECONDS	(int)0x02ca
#define	RAMMINUTES	(int)0x02cb
#define	RAMHOURS	(int)0x02cc
#define	RAMWEEKDAY	(int)0x02cd
#define	RAMDAY		(int)0x02ce
#define	RAMMONTH	(int)0x02cf
#define	INTSTATUS	(int)0x02d0
#define	INTCONTROL	(int)0x02d1
#define	COUNTRESET	(int)0x02d2
#define	RAMRESET	(int)0x02d3
#define	STATUS		(int)0x02d4
#define	GO		(int)0x02d5
#define	INTSTANDBY	(int)0x02d6
#define	TEST		(int)0x02df

/*
	The following are redefinitions of three of the above
	ports to match MSDOS ASTCLOCK routine's usage of ports.
	The redefinitions are only in name for clarity.
*/

#define	LASTMONTH	(int)0x02c9
#define	YEAR		(int)0x02ca
#define	SPECIAL		(int)0x02cb

/*
	The following define two special codes as used
	by AST software on the PC-DOS side.
	INIT tells whether there has ever been access
	to the AST card.  If the location for INIT
	(called SPECIAL - see above) has any other value
	it means that a total reset has been applied to
	the board, or battery has been changed, or this
	is a first access ever to the board.
	ALL is a byte with 8 bit-flags and is used to
	tell the clock chip on AST board how to reset
	its internal counters and/or ram buckets.
	See AST documentation for more.
*/

#define	INIT		(unsigned char)0xde
#define	ALL		(unsigned char)0xff

#define	CENTURY		1900

char	*week[8] =
	{
		"WEEK-ERROR",
		"Monday",
		"Tuesday",
		"Wednesday",
		"Thursday",
		"Friday",
		"Saturday",
		"Sunday"
	};

char	*month[13] =
	{
		"MONTH-ERROR",
		"January",
		"February",
		"March",
		"April",
		"May",
		"June",
		"July",
		"August",
		"September",
		"October",
		"November",
		"December"
	};

char	getport();

main(argc,argv)
int	argc;
char	*argv[];
{
	if (!strcmp(argv[0],"setdate"))
		exit (setdate(argc,argv));
	exit (getdate(argc,argv));
}

setdate(argc,argv)
int	argc;
char	*argv[];
{
register int	i = 0, len, fy = 0, fm = 0, fd = 0, fh = 0, fn = 0, fs = 0;
register int	yy = 0, mm = 0, dd = 0, hh = 0, nn = 0, ss = 0, ram = 0;
register int	ww = 0, wy = 0, wm = 0, wd = 0;
register char	t[3];

	if (argc > 1)
	{
		if (argv[1][0] == '-')
		{
			if (argv[1][1] == 'r')
			{
				argv++;
				ram = 1;
			}
			else
				return (usage(argv));
		}
	}
	else
		return (usage(argv));
	len = strlen(argv[1]);
	if (getuid() != 0)
	{
		printf("Must be root to set date\n");
		return (0);
	}
	if ((len > 13) || (len < 1))
		return (usage(argv));
	switch (len)
	{
		case 4:			/* just hours and minutes   	*/
			fh = fn = 1;
			break;
		case 6:			/* just year, month and day 	*/
			fy = fm = fd = 1;
			break;
		case 10:		/* all except seconds		*/
			fy = fm = fd = fh = fn = 1;
			break;
		case 13:		/* all				*/
			fy = fm = fd = fh = fn = fs = 1;
			break;
		default:		/* invalid parameters specified */
			return (usage(argv));
	}
	if ((i = getport(SPECIAL) & 0x00ff) != INIT)
	{
		if (!fy)
		{
			printf("Clock calendar is now reset - ");
			printf("specify full date and time\n\n");
			return (usage(argv));
		}
		putport(ALL,COUNTRESET);
		putport(ALL,RAMRESET);
		putport(INIT,SPECIAL);
	}
	if (fy)
	{
		t[0] = argv[1][0];
		t[1] = argv[1][1];
		t[2] = '\0';
		if (((yy = atoi(t)) < 80) || (yy > 99))
		{
			printf("year must be between 80 and 99\n");
			return (usage(argv));
		}
		wy = yy + CENTURY;
		yy -= 80;
		i = (yy % 10) & 0x000f;
		i |= (((yy  / 10) & 0x000f) << 4);
		putport((char)(i & 0x00ff),YEAR);
	}
	if (fm)
	{
		t[0] = argv[1][2];
		t[1] = argv[1][3];
		t[2] = '\0';
		if (((mm = atoi(t)) < 1) || (mm > 12))
		{
			printf("month must be between 01 and 12\n");
			return (usage(argv));
		}
		wm = mm;
		i = (mm % 10) & 0x000f;
		i |= (((mm  / 10) & 0x0001) << 4);
		putport((char)(i & 0x00ff),MONTH);
		if (ram)
			putport((char)(i & 0x00ff),RAMMONTH);
		if (mm-- < 1)
			mm = 12;
		i = (mm % 10) & 0x000f;
		i |= (((mm  / 10) & 0x0001) << 4);
		putport((char)(i & 0x00ff),LASTMONTH);
	}
	if (fd)
	{
		t[0] = argv[1][4];
		t[1] = argv[1][5];
		t[2] = '\0';
		if (((dd = atoi(t)) < 1) || (dd > 31))
		{
			printf("day must be between 01 and 31\n");
			return (usage(argv));
		}
		wd = dd;
		i = (dd % 10) & 0x000f;
		i |= (((dd  / 10) & 0x0003) << 4);
		putport((char)(i & 0x00ff),DAY);
		if (ram)
			putport((char)(i & 0x00ff),RAMDAY);
	}
	if (fy)
	{
		ww = day(wm, wd, wy);
		if (ww == 0)		/* adjust SUNDAY to AST format */
			ww = 7;
		putport((char)(ww & 0x0007),WEEKDAY);
		if (ram)
			putport((char)(ww & 0x0007),RAMWEEKDAY);
	}
	if (fh)
	{
		if (fy)
		{
			t[0] = argv[1][6];
			t[1] = argv[1][7];
		}
		else
		{
			t[0] = argv[1][0];
			t[1] = argv[1][1];
		}
		t[2] = '\0';
		if (((hh = atoi(t)) < 0) || (hh > 23))
		{
			printf("hours must be between 00 and 23\n");
			return (usage(argv));
		}
		i = (hh % 10) & 0x000f;
		i |= (((hh  / 10) & 0x0003) << 4);
		putport((char)(i & 0x00ff),HOURS);
		if (ram)
			putport((char)(i & 0x00ff),RAMHOURS);
	}
	if (fn)
	{
		if (fy)
		{
			t[0] = argv[1][8];
			t[1] = argv[1][9];
		}
		else
		{
			t[0] = argv[1][2];
			t[1] = argv[1][3];
		}
		t[2] = '\0';
		if (((nn = atoi(t)) < 0) || (nn > 59))
		{
			printf("minutes must be between 00 and 59\n");
			return (usage(argv));
		}
		i = (nn % 10) & 0x000f;
		i |= (((nn  / 10) & 0x0007) << 4);
		putport((char)(i & 0x00ff),MINUTES);
	}
	if (fs)
	{
		t[0] = argv[1][11];
		t[1] = argv[1][12];
		t[2] = '\0';
		if (((ss = atoi(t)) < 0) || (ss > 59))
		{
			printf("seconds must be between 00 and 59\n");
			return (usage(argv));
		}
		i = (ss % 10) & 0x000f;
		i |= (((ss  / 10) & 0x0007) << 4);
		putport((char)(i & 0x00ff),SECONDS);
	}
	putport((char)0,HUNDREDTHS);
	putport((char)0,THOUSANDTHS);
	putport((char)0,RAMTHOUSANDTHS);
	putport((char)0,INTCONTROL);
	return (1);
}

getdate(argc,argv)
int	argc;
char	*argv[];
{
	argc--, argv++;
	while (argc > 0 && **argv == '-')
	{
		(*argv)++;
		while (**argv)
		{
			switch (*(*argv)++)
			{
				case 'T':	/* time for date	  */
					  return (time());

				case 'D':	/* date for date	  */
					  return (date());

				case 't':	/* time for user	  */
					  return (utime());

				case 'd':	/* date for user	  */
					  return (udate());

				case 'a':	/* date and time for user */
					  return (udatetime());

				case 'A':	/* for debugging	  */
					  return (dumpall());

				case '-':	/* for help		  */
				case '?':	/* for help		  */
				case 'h':	/* for help		  */
					  return (help());
			}
                }
                argc--, argv++;
        }
	return (datetime());			/* date and time for date */
}

time()
{
register unsigned int c, hh, mm;

	c = getport(HOURS);
	hh = c & 0x000f;
	hh += ((c >> 4) & 0x0003) * 10;
	c = getport(MINUTES);
	mm = c & 0x000f;
	mm += ((c >> 4) & 0x0007) * 10;
	printf("%02d%02d",hh,mm);
	return (1);
}

date()
{
register unsigned int c, dd, mm, yy;

	c = getport(DAY);
	dd = c & 0x000f;
	dd += ((c >> 4) & 0x0003) * 10;
	c = getport(MONTH);
	mm = c & 0x000f;
	mm += ((c >> 4) & 0x0001) * 10;
	c = getport(YEAR);
	yy = c & 0x000f;
	yy += ((c >> 4) & 0x000f) * 10;
	printf("%02d%02d%02d",(yy + 80),mm,dd);
	return (1);
}

datetime()
{
	date();
	time();
	return (1);
}

utime()
{
register unsigned int c, hh, mm, ss, am;

	c = getport(HOURS);
	hh = c & 0x000f;
	hh += ((c >> 4) & 0x0003) * 10;
	if (hh < 12)
		am = 1;
	else
		am = 0;
	if ((hh %= 12) == 0)
		hh = 12;
	c = getport(MINUTES);
	mm = c & 0x000f;
	mm += ((c >> 4) & 0x0007) * 10;
	c = getport(SECONDS);
	ss = c & 0x000f;
	ss += ((c >> 4) & 0x0007) * 10;
	printf("The time is: %d:%02d:%02d %s\n",hh,mm,ss,(am ? "AM" : "PM"));
	return (1);
}

udate()
{
register unsigned int c, dd, mm, yy, ww;

	c = getport(DAY);
	dd = c & 0x000f;
	dd += ((c >> 4) & 0x0003) * 10;
	c = getport(MONTH);
	mm = c & 0x000f;
	mm += ((c >> 4) & 0x0001) * 10;
	c = getport(YEAR);
	yy = c & 0x000f;
	yy += ((c >> 4) & 0x000f) * 10;
	c = getport(WEEKDAY);
	ww = c & 0x0007;
	printf("The date is: %s - %s %d, 19%d PDT\n",
			week[ww],month[mm],dd,(yy + 80));
	return (1);
}

udatetime()
{
	udate();
	utime();
	return (1);
}

dumpall()
{
register unsigned int c, i;

	c = getport(THOUSANDTHS);
	i = ((c >> 4) & 0x000f) * 10;
	printf("ten thousandth of a second  = %d\n",i);

	c = getport(HUNDREDTHS);
	i = c & 0x000f;
	i += ((c >> 4) & 0x000f) * 10;
	printf("hundredth of a second       = %d\n",i);

	c = getport(SECONDS);
	i = c & 0x000f;
	i += ((c >> 4) & 0x0007) * 10;
	printf("seconds                     = %d\n",i);

	c = getport(MINUTES);
	i = c & 0x000f;
	i += ((c >> 4) & 0x0007) * 10;
	printf("minutes                     = %d\n",i);

	c = getport(HOURS);
	i = c & 0x000f;
	i += ((c >> 4) & 0x0003) * 10;
	printf("hours                       = %d\n",i);

	c = getport(WEEKDAY);
	i = c & 0x0007;
	printf("weekday                     = %d %s\n",i,week[i]);

	c = getport(DAY);
	i = c & 0x000f;
	i += ((c >> 4) & 0x0003) * 10;
	printf("day                         = %d\n",i);

	c = getport(MONTH);
	i = c & 0x000f;
	i += ((c >> 4) & 0x0001) * 10;
	printf("month                       = %d %s\n",i,month[i]);

	c = getport(RAMTHOUSANDTHS);
	i = ((c >> 4) & 0x000f) * 10;
	printf("upper - ram thousandths     = %d c = 0x%x %o\n",i,c,c);

	c = getport(RAMHUNDREDTHS);
	i = c & 0x000f;
	i += ((c >> 4) & 0x000f) * 10;
	printf("last month - ram hundredths = %d %s c = 0x%x %o\n",
							i,month[i],c,c);

	c = getport(RAMSECONDS);
	i = c & 0x000f;
	i += ((c >> 4) & 0x000f) * 10;
	printf("year - ram seconds          = 19%d c = 0x%x %o\n",(i + 80),c,c);
	
	c = getport(RAMMINUTES);
	i = c & 0x000f;
	i += ((c >> 4) & 0x0007) * 10;
	printf("special - ram minutes       = %d  c = 0x%x %o\n",i,c,c);

	c = getport(RAMHOURS);
	i = c & 0x000f;
	i += ((c >> 4) & 0x0003) * 10;
	printf("ram - hours                 = %d\n",i);

	c = getport(RAMWEEKDAY);
	i = c & 0x0007;
	printf("ram - weekday               = %d\n",i);

	c = getport(RAMDAY);
	i = c & 0x000f;
	i += ((c >> 4) & 0x0003) * 10;
	printf("ram - day                   = %d\n",i);

	c = getport(RAMMONTH);
	i = c & 0x000f;
	i += ((c >> 4) & 0x0001) * 10;
	printf("ram - month                 = %d\n",i);

	c = getport(INTSTATUS);
	printf("interrupt status            = %d\n",c);

	c = getport(INTCONTROL);
	printf("interrupt control           = %d\n",c);

	c = getport(COUNTRESET);
	printf("count reset                 = %d\n",c);

	c = getport(RAMRESET);
	printf("ram reset                   = %d\n",c);

	c = getport(STATUS);
	printf("status                      = %d\n",c);

	c = getport(GO);
	printf("go                          = %d\n",c);

	c = getport(INTSTANDBY);
	printf("interrupt standby           = %d\n",c);

	c = getport(TEST);
	printf("test                        = %d\n",c);

	return (1);
}

usage()
{
	printf("usage: setdate [-r] yymmddhhmm.ss\n");
	printf("       -r  -  set rams in addition to counters\n");
	printf("       yy  -  year\n");
	printf("       mm  -  month\n");
	printf("       dd  -  day\n");
	printf("       hh  -  hours\n");
	printf("       mm  -  minutes\n");
	printf("        .  -  necessary to indicate seconds\n");
	printf("       ss  -  seconds   (optional: default = 0)\n");
	return (0);
}

help()
{
	printf("usage: getdate [-T | -D | -t | -d | -a | -A | -h | -? | --]\n");
	printf("no options  -  returns 'yymmddhhmm' for date command\n");
	printf("        -T  -  returns 'hhmm' for date command\n");
	printf("        -D  -  returns 'yymmdd' for date command\n");
	printf("        -t  -  return time in user readable format\n");
	printf("        -d  -  return date in user readable format\n");
	printf("        -a  -  return date and time in user readable format\n");
	printf("        -A  -  dump all counters of the calendar clock\n");
	printf("        --  -  print this options list\n");
	printf("        -?  -  print this options list\n");
	printf("        -h  -  print this options list\n");
	return (0);
}

/*
	The following routine returns an integer 0 thru 6 for
	day of week ( 0 = Sunday and 6 = Saturday ).
	It was borrowed from the net (I forget the name of writer)
	so it is not of my creation.
*/

day(m, d, y)
int	m, d, y;
{
int	y0, y1, y2, m1, dbuf;
 
	m1 = ((m + 9) % 12) + 1;
	y0 = (m <= 2) ? (y - 1) : y;
	y1 = y0 / 100;
	y2 = y0 % 100;
	dbuf = ((26 * m1 - 2) / 10 + d + y2 + y2 / 4 + y1 / 4 - 2 * y1) % 7;
	if (dbuf < 0)
		dbuf += 7;
	return (dbuf);
}

/*
	The following routines go directly to the hardware
	ports bypassing all the bios and everything else.
	You can use them to poke and peek any other port.
	They are highly optimized (as fas as I know) and
	very sensitive about additions to them as far as
	parameters and/or additional lines of code.
	SO, LEAVE THEM ALONE FOR YOUR PROTECTION.
*/

char
getport(p)
{
	asm("	mov	dx,4(bp)");
	asm("	in");
	asm("	mov	ah,#0");
}

putport(c,p)
{
	asm("	mov	dx,6(bp)");
	asm("	mov	ax,*4(bp)");
	asm("	out");
}
/*-----------------------------------THE END-----------------------------*/
