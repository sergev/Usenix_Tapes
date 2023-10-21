/* Copyright 1985 NCR Corporation - Dayton, Ohio, USA */

static char _Version[] = "%W%  Compiled: %T% %H%  Delta Date: %U% %G%";
#include "sys/types.h"
#include "sys/times.h"
#include "stdio.h"
#include "termio.h"
#define MAXPROC 64

#define NULL 0


static int ttyfd;
static double timex;
static struct termio ttystru;


static int noproc,maxjobs;
static int nomp,nottyc,nottycl;
static int count,itotal,status;
static int pid,cid,tagid;
static double stime;
static char tty[MAXPROC][14],s[100];
static int tablepid[MAXPROC];
static int notty,nochar,tnotty,tnochar,nodiskr,nouser;
static double jobs;
static long start,end,ftime;


FILE * stream;
static char line[32],buffer[1024],pbuffer[32],bufin[1024];
static char namev[8][80];
static double pcnt[8],sum;
static int novol,fsum,nfile[8],lng,len[8];
/* tty open */

ttyopen( )
{
	
	if( (ttyfd = open(&tty[tagid][0],1)) < 0 )
	{
		printf("can not open %s for tty write\n",&tty[tagid][0]);
		return;
	}
	ioctl(ttyfd,TCGETA,&ttystru);
	ttystru.c_cflag = B9600 | CS7 | CREAD | PARENB | CLOCAL;
	ttystru.c_oflag = OPOST | ONLCR | TAB3;
	ioctl(ttyfd,TCSETA,&ttystru);
	return;
}

main(argc,argv)
	int argc;
	char * argv[ ];
{
	register int i,j,k;

	if( argc != 3 )
	{
		printf("incorrect number of arguments for run\n");
		exit(1);
	}
	for(k=0; k<31; k++)
	{
		line[k] = 'A' + k % 10;
		pbuffer[k] = 'K' + k % 10;
	}
	line[31] = '\n';
	pbuffer[31] = '0';
	if( (stream = fopen("config","r")) == NULL )
	{
		printf("unable to open file config\n");
		exit(1);
	}
	i = 1;
	while( i <=3 )
	{
		if( fgets(s,100,stream) == NULL )
		{
			printf("comfig file is empty\n");
			exit(1);
		}
		if( s[0] == '#' ) continue;
		switch(i)
		{
		case 1:
			sscanf(s,"%d",&nomp);
			break;
		case 2:
			sscanf(s,"%d",&noproc);
			break;
		case 3:
			sscanf(s,"%d",&novol);
			break;
		}
		i++;
	}
	for(i=0; i<novol; i++)
	{
		lab2:
		if( fgets(s,100,stream) == NULL)
		{
			printf("not enough disk volume names\n");
			exit(1);
		}
		if( s[0] == '#' ) goto lab2;
		sscanf(s,"%f %s",&pcnt[i],&namev[i][0]);
	}
	i = 1;
	while( i <=2 )
	{
		if( fgets(s,100,stream) == NULL )
		{
			printf("comfig file is empty\n");
			exit(1);
		}
		if( s[0] == '#' ) continue;
		switch(i)
		{
		case 1:
			sscanf(s,"%d",&nottyc);
			break;
		case 2:
			sscanf(s,"%d",&nottycl);
			break;
		}
		i++;
	}
	j = 0;
	while( fgets(s,100,stream) != NULL )
	{
		if( s[0] == '#' ) continue;
		if( ++j > MAXPROC )
		{
			printf("too many entries in config file\n");
			exit(1);
		}
		sscanf(s,"%s",&tty[j][0]);
	}
	fclose(stream);
	sscanf(argv[1],"%d",&notty);
	sscanf(argv[2],"%d",&nodiskr);
	maxjobs = 200;
	ptime(1);
	count = 0;
	itotal = 0;
	for(i=1; i<=noproc; i++)
	{
		tablepid[i] = 0;
	}
	i = -1;
	start = ptime(1);
	while( 1 )
	{
		i++;
		tagid = getid( );
		if( ( pid = fork( ) ) == 0 )
		{
			if( notty )
				ttyopen( );
			for(i=1; i<=notty; i++)
			{
				if( write(ttyfd,line,sizeof line) != sizeof line)
				{
					printf(" can not write %s on tty write\n",&tty[tagid][0]);
					exit(1);
				}
			}
			if( notty )
				close(ttyfd);
			exit(0);
		}
		if( pid > 0 )
		{
			insert(pid);
			count++;
		}
		else
		{
			printf(" can not fork child process\n");
			exit(1);
		}
		if( count == noproc )
		{
			cid = wait(&status);
			count--;
			freeit(cid);
			if( ++itotal >= (maxjobs + 1 - noproc) )
			{
				while( count > 0 )
				{
					cid = wait(&status);
					count--;
					itotal++;
					freeit(cid);
				}
				goto ENDIT;
			}
		}
	}
	ENDIT:
	end = ptime(1);
	if( (stream = fopen("answers","a")) == NULL )
	{
		printf("unable to open file answers\n");
		exit(1);
	}
	ftime = end - start;
	timex = ( (double)ftime ) / 1000.0;
	jobs = ((double)maxjobs * 60.0) / timex;
	fprintf(stream,"%3d %3d %9.3f %9.3f\n",
		2000,0,jobs,timex);
	fclose(stream);
	if( nodiskr )
	{
		if( (stream = fopen("subele","a")) == NULL )
		{
			printf("unable to open file subele\n");
			exit(1);
		}
		nochar = notty * 32;
		tnotty = maxjobs * notty;
		tnochar = maxjobs * nochar;
		fprintf(stream,"\n  T T Y   S U B S Y S T E M\n");
		fprintf(stream," Total  Characters Written per Second %9.3f\n",
				(double)tnochar / timex);
		fprintf(stream," Characters Written per Second per Line %9.3f\n",
				(double)tnochar / ((double)noproc * timex ) );
		fprintf(stream," Total Characters Written %d request size %d \n",
				tnochar,32);
		fprintf(stream," Total Time To Write Characters %9.3f Seconds\n",timex);
		fprintf(stream," Number of TTY Controllers %d\n",nottyc);
		fprintf(stream," Number of TTY Lines per Controller %d\n",nottycl);
		fprintf(stream," Number of TTY Lines Used in Run %d\n",noproc);
		fprintf(stream," Jobs per Minute %9.3f\n",(double)(60 * maxjobs) / timex);
		fclose(stream);
	}
	exit(0);
}

long
ptime(tflag)
	int tflag;
{
	static long tm[4];
	static int first = 1;
	static long tstart = 0;
	static long xtime = 0;
	long times( );
	register int i;

	if( tflag && first )
	{
		tstart = times(tm);
		first = 0;
		return(0);
	}
	xtime = times(tm);
	if( tflag )
	{
		xtime = (1000 * (xtime - tstart)) / 60;
	}
	else
	{
		xtime = (1000 * tm[0] ) / 60;
	}
	return(xtime);
}

insert(pid)
	int pid;
{
	register int j;

	for(j=1; j<=noproc; j++)
	{
		if( tablepid[j] == 0 )
		{
			tablepid[j] = pid;
			return(pid);
		}
	}
	printf("no free space on insert pid\N");
	return(0);
}

freeit(pid)
	int pid;
{
	register int j;

	for(j=1; j<=noproc; j++)
	{
		if( tablepid[j] == pid )
		{
			tablepid[j] = 0;
			return(j);
		}
	}
	printf("pid not found in table on free\n");
	return(0);
}

getid( )
{
	register int j;

	for(j=1; j<=noproc; j++)
	{
		if( tablepid[j] == 0 )
		{
			return(j);
		}
	}
	printf("no free space on getpid\n");
	return(0);
}
