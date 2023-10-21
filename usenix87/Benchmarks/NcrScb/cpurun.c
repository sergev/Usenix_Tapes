/* Copyright 1985 NCR Corporation - Dayton, Ohio, USA */

static char _Version[] = "%W%  Compiled: %T% %H%  Delta Date: %U% %G%";
#include "stdio.h"
#define MAXPROC 64

#define NULL 0

FILE * stream;

static double timex,timey;


static int noproc,maxjobs;
static int nomp,nottyc,nottycl;
static int count,itotal,status;
static int pid,cid,tagid;
static double stime;
static int tablepid[MAXPROC];
static int notty,nodiskw,nodiskr,nouser;
static double jobs;
static long start,end,ftime;


main( )
{
	register int i,dc,tc;
	static int k,j,n,nx;
	static double ddcnt,ttcnt;
	static double dstep,ddelta,tstep,tdelta;

	maxjobs = 200;
	ptime(1);
	count = 0;
	itotal = 0;
	noproc = 1;
	for(i=1; i<=noproc; i++)
	{
		tablepid[i] = 0;
	}
	if( (stream = fopen("loopcount","r")) == NULL )
	{
		printf("unable to open file loopcount\n");
		exit(0);
	}
	fscanf(stream,"%d",&nx);
	fclose(stream);
	nx--;
	notty = 30;
	nodiskr = 32;
	dc = 1;
	tc = 1;
	dstep = 0.0;
	ddelta = (double)nodiskr / (double)nx;
	tstep = 0.0;
	tdelta = (double)notty / (double)nx;
	nx++;
	i = -1;
	start = ptime(1);
	while( 1 )
	{
		i++;
		tagid = getid( );
		if( ( pid = fork( ) ) == 0 )
		{
			for(i=1; i<=nx; i++)
			{
				dstep += ddelta;
				ddcnt = (double)dc;
				if( dstep >= ddcnt )
				{
					dc++;
				}
				tstep += tdelta;
				ttcnt = (double)tc;
				if( tstep >= ttcnt )
				{
					tc++;
				}
			}
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
	ftime = end - start;
	timex = ( (double)ftime ) / 1000.0;
	count = 0;
	itotal = 0;
	noproc = 8;
	for(i=1; i<=noproc; i++)
	{
		tablepid[i] = 0;
	}
	nx--;
	notty = 30;
	nodiskr = 32;
	dc = 1;
	tc = 1;
	dstep = 0.0;
	ddelta = (double)nodiskr / (double)nx;
	tstep = 0.0;
	tdelta = (double)notty / (double)nx;
	nx++;
	i = -1;
	start = ptime(1);
	while( 1 )
	{
		i++;
		tagid = getid( );
		if( ( pid = fork( ) ) == 0 )
		{
			for(i=1; i<=nx; i++)
			{
				dstep += ddelta;
				ddcnt = (double)dc;
				if( dstep >= ddcnt )
				{
					dc++;
				}
				tstep += tdelta;
				ttcnt = (double)tc;
				if( tstep >= ttcnt )
				{
					tc++;
				}
			}
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
				goto ENDIT2;
			}
		}
	}
	ENDIT2:
	end = ptime(1);
	ftime = end - start;
	timey = ( (double)ftime ) / 1000.0;
	if( (stream = fopen("subele","a")) == NULL )
	{
		printf("unable to open file subele\n");
		exit(1);
	}
	fprintf(stream," Effective CPU Processing Power %9.5f\n",timex / timey);
	fclose(stream);
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
