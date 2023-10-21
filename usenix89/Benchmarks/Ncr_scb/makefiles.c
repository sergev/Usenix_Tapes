/* Copyright 1985 NCR Corporation - Dayton, Ohio, USA */

static char _Version[] = "%W%  Compiled: %T% %H%  Delta Date: %U% %G%";
#include <stdio.h>

FILE * stream;

static char bufferw[5120],bufferr[5120];
static char s[100],namev[8][80];
static double pcnt[8];
static int fdr,novol,lng;
static int nomp,noproc;

main( )
{
	register int i,k,j;
	register char * pw;
	register char * pr;

	pw = bufferw;
	pr = bufferr;
	for(i=0; i<5120; i++);
	{
		*pw++ = 'w';
		*pr++ = 'r';
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
	fclose(stream);
	for(i=0; i<novol; i++)
	{
		lng = strlen(&namev[i][0]);
		if( namev[i][lng-1] != '/')
		{
			namev[i][lng++] = '/';
		}
		namev[i][lng++] = 'd';
		namev[i][lng++] = 'k';
		namev[i][lng+3] = '\0';
		for(j=0; j<=199; j++)
		{
			sprintf(&namev[i][lng],"%03d",j);
			if( (fdr = creat(&namev[i][0],0666)) < 0 )
			{
				printf("can not open %s file\n",
					&namev[i][0]);
				exit(1);
			}
			for(k=1; k<=4; k++)
			{
				if( write(fdr,bufferr,5120) != 5120 )
				{
					printf("unable to write read buffer %3d\n",i);
					exit(1);
				}
			}
			close(fdr);
			printf("name = %s\n",&namev[i][0]);
		}
	}
}
