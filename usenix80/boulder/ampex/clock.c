#include <stdio.h>
#include <sgtty.h>
/*
 * Clock [alarm-time] [-s]
 *
 * displays hour:minute in banner fashsion, in 12-hour format.
 * minutes change on the minute (plus a second or two if loaded).
 * beeps each minute after the optional alarm time (given in 12-hour format).
 * will display each second (approx) if - arg given (heavy cpu user).
 *
 * retrieves TERM from the environment and looks up cursor motion
 * capability in /etc/termcap using the Berkeley library:
 *
 * cc clock.c -ltermlib
 *
 *	John Nickolls, Ampex Corp., Jan. 1980.
 */

char *usage = "usage: clock [alarm-hour:min]\n";

#define X	17	/* left-most column */
#define Y	10	/* upper-most row */
#define NBELL	1
#define BELL	007
char *num[]	/* block number patterns */
   {
	" ### ","#   #","#   #","#   #","#   #","#   #"," ### ", /* 0 */
	"  #  "," ##  ","  #  ","  #  ","  #  ","  #  "," ### ", /* 1 */
	" ### ","#   #","    #","    #"," ### ","#    ","#####", /* 2 */
	" ### ","#   #","    #","  ## ","    #","#   #"," ### ", /* 3 */
	"#    ","#  # ","#  # ","#  # ","#####","   # ","   # ", /* 4 */
	"#####","#    ","#    ","#### ","    #","#   #"," ### ", /* 5 */
	" ### ","#   #","#    ","#### ","#   #","#   #"," ### ", /* 6 */
	"#####","    #","   # ","  #  ","  #  ","  #  ","  #  ", /* 7 */
	" ### ","#   #","#   #"," ### ","#   #","#   #"," ### ", /* 8 */
	" ### ","#   #","#   #"," ####","    #","#   #"," ### ", /* 9 */
	"#####"
  };
char *colon[]

   {
	"  ","[]","[]","  ","[]","[]","  ",
	"[]"
   };
char tbuf[BUFSIZ];		/* for termcap routines */
char area[200];			/* area for characters for termcap */
char *areap = &area[0];		/* ptr to character area */
char PC, *BC, *UP;
char *tcm, *tcl, *tpc;
int tli;
short ospeed;
struct sgttyb ttyb;

main(argc,argv)
int argc; char **argv;
   {
	int *ptr, *localtime();
	int sec, min, hrs;
	int amin, ahrs;
	int stime, stime1, tvec[2];
	int i, alarm, everysec;
	char *cptr;
	int tputchar();

	if (gtty(1, &ttyb) == -1)
		exit(1);
	ospeed = ttyb.sg_ospeed;
	if (tgetent(tbuf, getenv("TERM")) <= 0)
	  {	printf("clock: no termcap entry\n");
		exit(1);
	  }
	if ((tcm = tgetstr("cm", &areap)) == NULL)
	  {	printf("clock: no cursor motion capability\n");
		exit(1);
	  }
	if ((tcl = tgetstr("cl", &areap)) == NULL)
	  {	printf("clock: no clear screen capability\n");
		exit(1);
	  }
	UP = tgetstr("up", &areap);
	BC = tgetstr("bc", &areap);
	if (tpc = tgetstr("pc", &areap))
		PC = tpc[0];
	if ((tli = tgetnum("li")) == -1)
	  {	printf("clock: not a crt\n");
		exit(1);
	  }
	stime= 60;
	stime1 = 0;
	everysec= 0;
	while( --argc )
	   {
		argv++;
		if( argv[0][0] == '-' )
		   {
			if (argv[0][1] == 's')
			  {
				stime = 1;
				everysec++;
			  }
			else
			  {
				printf(usage);
				exit(1);
			  }
		   }
		 else
		   {
			ahrs= atoi(*argv);
			for(cptr= *argv; *cptr && (*cptr != ':'); cptr++)
			  {	if (*cptr < '0' || *cptr > '9')
				  {	printf(usage);
					exit(1);
				  }
			  }
			if(*cptr == ':') cptr++;
			amin= atoi(cptr);
			alarm++;
		   }
	   }
	tputs(tcl, 25, &tputchar);	/* clear screen */

	for(;;)
	   {
		time(tvec);
		ptr= localtime(tvec);
		sec= *ptr++;
		min= *ptr++;
		hrs= *ptr;
		if (hrs >= 12) hrs -= 12;	/* hrs = 0-11 */
		if (stime1 == 0)	/* 1st loop */
		  {	if (stime == 60)
				stime1 = 61 - sec;	/* align on minute */
			if (alarm)
			  {	ahrs %= 12;
				move(40, 2);
				printf("Alarm set for %02d:%02d.",
					(ahrs == 0? 12: ahrs), amin);
				if (hrs > ahrs)	/* cross 12 */
					ahrs += 12;	/* alarm <= 12 hrs */
				if (hrs == ahrs && min > amin)
					ahrs += 12;
			  }
		  }
		if (alarm)
		  {	if ((ahrs - hrs) > 12)	/* hrs crossed 12 */
				ahrs -= 12;
			if(hrs >= ahrs && min >= amin)
				for(i=0;i<NBELL;i++) putchar(BELL);
		  }
		if (hrs == 0) hrs = 12;	/* for clock-like display */
		for(i=0; i<7; i++)
		   {
			move(X,Y+i);
			printf("  %s   %s   %s   %s   %s",
				num[(hrs/10)*7+i],
				num[(hrs%10)*7+i],
				colon[i],
				num[(min/10)*7+i],
				num[(min%10)*7+i]);
			if(everysec)
				printf("   %s   %s   %s",
					colon[i],
					num[(sec/10)*7+i],
					num[(sec%10)*7+i]);
		   }
		move(0, tli - 1);
		sleep(stime1);
		stime1 = stime;
	   }
   }

move(x,y)
  int x,y;
   {
	fputs(tgoto(tcm, x, y), stdout);
   }
tputchar(c)
  int c;
  {
	putchar(c);
  }
