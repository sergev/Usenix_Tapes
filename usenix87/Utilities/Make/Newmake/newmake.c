#ifndef lint
static char *newmake_c = "@(#)newmake.c	1.4 85/12/14 [marc@fcomp]";
#endif

/*
 * File name:		newmake.c 1.4
 * Created:		Saturday, December 4, 1985
 *      By:             Marc A. Ries (FCOMP, Anaheim Hills, CA)
 *
 * Latest change:	85/12/18 19:54:44
 * Purpose of this file:
 * 
 *    create a new Makefile for a single program
 * 
 */

#include "stdio.h"
#include <sys/types.h>

#define MAKE    "Makefile"		/* output file name */
#define MODE    0600 			/* -rw-r--r--       */

# ifdef DAYTIME 
/*
 * "Created: Wednesday, December 11, 1985" 
 */
#include <sys/times.h>			/* system dependent */
#include <time.h>			/* system dependent */

char   *dtime (), 
       *dasctime (), 
       *dtimezone (), 
       *dtimenow ();

struct tws *dlocaltime (), 
           *dparsetime ();
# endif DAYTIME 


FILE    *fp;
main(argc,argv)
int     argc;
int     **argv;
{
	char	*getenv ();
	/* the following should be exported shell variables */
	char	*NAME = getenv ("NAME");
	char	*ORG  = getenv ("ORGANIZATION");

	if (argc <= 1)		/* no command-line arguments */
		usage();
	if((fp=fopen("Makefile","w"))==0){
		fprintf(stderr,"  %s: can not open %s\n", argv[0], MAKE);
		exit(1);
	}

	fprintf(fp,"#\n");
	fprintf(fp,"# Makefile: %s\n", argv[1]);
# ifdef DAYTIME
	fprintf(fp,"#  Created: %s\n", dtimenow());
# endif DAYTIME
	if (NAME)
		fprintf (fp,"#       By: %s (%s)\n", NAME, ORG ? ORG : "");
	fprintf(fp,"#\n");
	fprintf(fp,"CC = /bin/cc\n");
	fprintf(fp,"CFLAGS = -O -s # Site Dependent\n");
	fprintf(fp,"SRCS = %s.c\n", argv[1]); 
	fprintf(fp,"OBJS = %s.o\n", argv[1]); 
	fprintf(fp,"\n");
	fprintf(fp,"all:\t%s\n", argv[1]); 
	fprintf(fp,"\n");
	fprintf(fp,"%s:\t$(OBJS)\n", argv[1]);
	fprintf(fp,"\t$(CC) $(CFLAGS) -o %s $(OBJS)\n", argv[1]);
	fprintf(fp,"\n");
	fprintf(fp,"clean:\n"); 
	fprintf(fp,"\trm -f $(OBJS) a.out core\n");
	fprintf(fp,"\n");
	fprintf(fp,"shar:\n");
	fprintf(fp,"\tshar -v %s.[clyh] Makefile > %s.shr\n", argv[1], argv[1]);
	fprintf(fp,"\n");
	fprintf(fp,"edit:\n");
	fprintf(fp,"\tvi $(SRCS)\n");
	fprintf(fp," \n");
	fprintf(fp,"%s.o:\t%s.c\n", argv[1], argv[1]);
	printf("  %s: A %s has been produced for %s\n", argv[0], MAKE, argv[1]);
	exit(0);
}

usage()
{
	printf("  Usage:	newmake filename\n");
	printf("  Output:	Makefile for the file filename.c\n");
	exit(1);
}

# ifdef DAYTIME
/* 
 * ala dtime.c - routines to do time structures 
 */

struct tws {    /* see <time.h> */
	int     tw_sec;
	int     tw_min;
	int     tw_hour;
	int     tw_mday;
	int     tw_mon;
	int     tw_year;
	int     tw_wday;
	int     tw_yday;
	int     tw_isdst;
};

static char *month[] = {
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

static char *day[] = {
	"Sunday", 
        "Monday", 
        "Tuesday", 
        "Wednesday", 
        "Thursday", 
        "Friday", 
        "Saturday"
};

struct keywd {
	char   *key;
	int     value;
};

long    time ();
struct tm *localtime ();

char   *dtime (clock)
long   *clock;
{
	return dasctime (dlocaltime (clock));
}

char *dtimenow()
{
	long    clock;
	time (&clock);
	return dtime (&clock);
}

struct tws *dlocaltime (clock)
long   *clock;
{
	struct tm  *tm;
	static struct tws   tw;

	if (!clock)
		return NULL;

	tm = localtime (clock);
	tw.tw_sec = tm -> tm_sec;
	tw.tw_min = tm -> tm_min;
	tw.tw_hour = tm -> tm_hour;
	tw.tw_mday = tm -> tm_mday;
	tw.tw_mon = tm -> tm_mon;
	tw.tw_year = tm -> tm_year;
	tw.tw_wday = tm -> tm_wday;
	tw.tw_yday = tm -> tm_yday;
	return (&tw);
}

char   *dasctime (tw)
struct tws *tw;
{
	static char buffer[80],
	result[80];

	if (!tw)
		return NULL;

	sprintf (buffer, "%s %02d, 19%02d",
	month[tw -> tw_mon], tw -> tw_mday, tw -> tw_year);
	sprintf (result, "%s, %s", day[tw -> tw_wday], buffer);
	return result;
}
# endif DAYTIME
