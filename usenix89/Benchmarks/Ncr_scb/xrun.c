/* Copyright 1985 NCR Corporation - Dayton, Ohio, USA */

static char _Version[] = "%W%  Compiled: %T% %H%  Delta Date: %U% %G%";
#include "sys/types.h"
#include "sys/times.h"
#include "stdio.h"
#include "termio.h"
#define MAXPROC 64

#define NULL 0


static int ttyfd,diskrfd,diskwfd,toggle;
static double timex;
static struct termio ttystru;


static int noproc,maxjobs;
static int nomp,nottyc,nottycl;
static int count,itotal,status;
static int pid,cid,tagid;
static double stime;
static char tty[MAXPROC][14],s[100];
static int tablepid[MAXPROC];
static int notty,nodiskw,nodiskr,nouser;
static double jobs;
static long start,end,ftime;


FILE * stream;
static char line[32],buffer[1024],pbuffer[32],bufin[1024];
static char namev[8][80];
static double pcnt[8],sum;
static int novol,fsum,nfile[8],lng,len[8];
static int total,ll[8],hl[8],cnt[8],vol,disk[200];
static int dist,sdist[8],oldf[8];
static int file1[200] =
{
	38,113,51,10,12,149,84,25,89,137,
	166,95,167,31,82,124,194,52,154,161,
	28,85,168,165,162,153,44,163,22,57,
	79,151,70,48,29,6,138,14,116,181,
	143,33,160,71,136,164,129,75,114,88,
	2,34,182,186,106,53,73,193,148,120,
	156,169,195,191,126,86,11,54,121,39,
	185,125,15,176,144,19,123,65,173,13,
	99,16,127,46,139,78,74,171,152,192,
	122,128,170,41,175,172,145,115,61,45,
	103,72,100,7,130,142,131,155,94,47,
	80,146,92,90,32,132,17,49,189,196,
	96,174,93,179,50,197,133,81,177,178,
	62,134,55,108,83,101,18,135,30,105,
	140,35,188,36,119,198,109,77,111,76,
	150,190,157,87,37,147,43,199,180,117,
	141,158,9,97,0,183,91,98,104,40,
	20,159,63,184,187,102,1,3,64,4,
	5,8,107,21,110,58,56,59,23,68,
	24,60,26,112,27,66,42,118,67,69
};
static int file2[200] =
{
	158,115,27,19,86,167,60,143,183,166,
	178,111,54,145,136,105,102,67,53,96,
	188,144,6,3,34,120,4,118,127,146,
	184,94,59,157,106,190,116,62,25,134,
	22,137,179,1,61,5,44,93,139,121,
	171,117,15,55,174,48,35,110,10,49,
	162,197,112,199,107,182,0,165,180,7,
	75,65,71,97,98,87,149,56,101,159,
	68,198,113,168,189,26,187,17,191,83,
	147,185,160,176,181,85,89,114,28,99,
	70,129,100,8,57,74,23,24,16,76,
	169,177,51,163,33,108,77,148,175,192,
	135,40,36,92,193,69,194,103,186,80,
	195,196,37,63,2,9,29,109,11,12,
	78,13,42,14,142,18,119,82,20,72,
	79,138,164,30,132,156,81,21,161,31,
	32,38,39,41,64,84,104,43,122,45,
	66,150,130,46,125,47,50,52,123,126,
	88,58,170,73,90,91,131,95,124,128,
	133,140,141,151,152,153,154,155,172,173
};
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


/* disk open */

dkopen(n)
	int n;
{
	register int d,f1,f2,lng;

	d = disk[n];
	f1 = file1[n];
	f2 = file2[n];
	lng = len[d];

	sprintf(&namev[d][lng],"%03d",f1);
	if( (diskrfd = open(&namev[d][0],0)) < 0 )
	{
		printf("can not open %s for disk read\n",&namev[d][0]);
		return;
	}
	sprintf(&namev[d][lng],"%03d",f2);
	if( (diskwfd = open(&namev[d][0],1)) < 0 )
	{
		printf("can not open %$ for disk write\n",&namev[d][0]);
		return;
	}
	return;
}
main(argc,argv)
	int argc;
	char * argv[ ];
{
	register int i,dc,tc;
	static int k,j,n,nx;
	static double ddcnt,ttcnt;
	static double dstep,ddelta,tstep,tdelta;

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
	for(k=0; k<1024; k++)
	{
		buffer[k] = 'A';
	}
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
	sum = 0.0;
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
		sum += pcnt[i];
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
	fsum = 0;
	for(i=0; i<novol; i++)
	{
		nfile[i] = (int)( ( (pcnt[i] * 200.0) / sum ) );
		fsum += nfile[i];
	}
	for(i=0; i<novol; i++)
	{
		if( fsum < 200 )
		{
			nfile[i]++;
			fsum++;
		}
		else
		{
			break;
		}
	}
	for(i=0; i<novol; i++)
	{
		lng = strlen(&namev[i][0]);
		if( namev[i][lng-1] != '/')
		{
			namev[i][lng++] = '/';
		}
		namev[i][lng++] = 'd';
		namev[i][lng++] = 'k';
		len[i] = lng;
		namev[i][lng+3] = '\0';
		sprintf(&namev[i][len[i]],"%03d",i);
	}
	total = 0;
	for(i=0; i<novol; i++)
	{
		cnt[i] = 0;
		oldf[i] = 0;
		sdist[i] = 0;
		ll[i] = total;
		total += nfile[i];
		hl[i] = total - 1;
	}
	for(j=0; j<200; j++)
	{
		k = rand( ) % 200;
		for(i=0; i<novol; i++)
		{
			if( (k>=ll[i]) && (k<=hl[i]) )
			{
				vol = i;
				break;
			}
		}
		disk[j] = vol;
	}
	ptime(1);
	count = 0;
	itotal = 0;
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
	dc = 1;
	tc = 1;
	dstep = 0.0;
	ddelta = (double)nodiskr / (double)nx;
	tstep = 0.0;
	tdelta = (double)notty / (double)nx;
	nx++;
	k = 1;
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
			if( nodiskr )
				dkopen(i);
			toggle = 1;
			for(i=1; i<=nx; i++)
			{
				dstep += ddelta;
				ddcnt = (double)dc;
				if( dstep >= ddcnt )
				{
					dc++;
					if( toggle )
					{
						if( read(diskrfd,bufin,sizeof bufin)
							!= sizeof bufin )
						{
							printf("can not read %s on disk read\n",&namev[disk[i]][0]);
							exit(1);
						}
						toggle = 0;
					}
					else
					{
						if( write(diskwfd,buffer,sizeof buffer)
							!= sizeof buffer )
						{
							printf("can not write %s on on disk write\n",&namev[disk[i]][0]);
							exit(1);
						}
						toggle = 1;
					}
				}
				tstep += tdelta;
				ttcnt = (double)tc;
				if( tstep >= ttcnt )
				{
					tc++;
					if( write(ttyfd,line,sizeof line) != sizeof line)
					{
						printf(" can not write %s on tty write\n",&tty[tagid][0]);
						exit(1);
					}
				}
			}
			if( notty )
				close(ttyfd);
			if( nodiskr )
			{
				close(diskrfd);
				close(diskwfd);
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
	if( (stream = fopen("answers","a")) == NULL )
	{
		printf("unable to open file answers\n");
		exit(1);
	}
	ftime = end - start;
	timex = ( (double)ftime ) / 1000.0;
	jobs = ((double)maxjobs * 60.0) / timex;
	fprintf(stream,"%3d %3d %9.3f %9.3f\n",
		notty,nodiskr,jobs,timex);
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
