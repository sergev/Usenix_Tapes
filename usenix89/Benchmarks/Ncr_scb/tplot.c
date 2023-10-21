/* Copyright 1985 NCR Corporation - Dayton, Ohio, USA */

static char _Version[] = "%W%  Compiled: %T% %H%  Delta Date: %U% %G%";
#include <stdio.h>

static int noproc;
static int notty,nodiskr,toggle;
static char screen[51][111],c[5],s[101],s1[100];
static double xmax,xmin,ymax,ymin;
static int xi[10][50],yi[10][50];
static double x[10][50],y[10][50];
static int tagv,xv;
static char tagc[11] =
{"DTABCEFGHI"};
static int flag = 1;
static int tagi[10] =
{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
static int tagh = -1;
static int tag[10] =
{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
static double xu,yu,xd,yd;
static float jobs;
FILE * stream;
static char ylab[11][8],xlab[10][8];
static char labtemp[10];
static char xaxis[92] = {
"+---------+---------+---------+---------+---------+---------+---------+---------+---------+"};
static char yaxis[42] = {
"+|||+|||+|||+|||+|||+|||+|||+|||+|||+|||+"};
static char letters[27] = {
"abcdefghijklmnopqrstuvwxyz"};
static char f = '\f';
static char var1[ ] =
{"             J = THROUGHPUT - JOBS PER MINUTE"};
static char var2[ ] =
{"           DISK = DISK READ + WRITE REQUEST PER JOB"};
static char var3[ ] =
{"           T = TTY WRITE REQUEST PER JOB"};
static char title[100];

main(argc,argv)
	int argc;
	char * argv[ ];
{
	register int i,n,j,j1,j2,k;

	if( argc != 2 )
	{
		printf("incorect number of arguments\n");
		exit(1);
	}
	if( (stream = fopen(argv[1],"r")) == NULL )
	{
		printf("unable to open file %s\n",argv[1]);
		exit(2);
	}
	/* read in title */
	if( fgets(title,100,stream) == NULL )
	{
		printf("The file %s is empty\n",argv[1]);
		exit(1);
	}
	n = strlen(title);
	title[n-1] = '\0';
	if( fgets(s,100,stream) == NULL )
	{
		printf("The file %s is empty\n",argv[1]);
		exit(1);
	}
	sscanf(s,"%d",&noproc);
	/* clear screen */
	for(i=0; i<=50; i++)
	{
		for(j=0; j<=110; j++)
		{
			screen[i][j] = ' ';
			if( (i==0) || (i==50) )
				screen[i][j] = '*';
			if( (j==0) || (j==109) )
				screen[i][j] = '*';
			if( j == 110 )
				screen[i][j] = '\0';
		}
	}
	/* put in x and y axis */
	j = 13;
	j1 = 1;
	for(i=0; i<=40; i++)
	{
		screen[j1++][j] = yaxis[i];
	}
	i = 42;
	k = 0;
	j = 14;
	while( j <= 104 )
	{
		screen[i][j] = xaxis[k++];
		j++;
	}
	/* read in y and x values */
	i = 0;
	while( fgets(s,100,stream) != NULL )
	{
		sscanf(s,"%d %d %f",&notty,&nodiskr,&jobs);
		if(flag)
		{
			tagv = notty;
			xv = nodiskr;
		}
		else
		{
			tagv = nodiskr;
			xv = notty;
		}
		for(i=0; i<=tagh; i++)
		{
			if(tagv == tag[i])
			{
				j = i;
				goto lab1;
			}
		}
		j = ++tagh;
		tag[j] = tagv;
	lab1:
		k = ++tagi[j];
		x[j][k] = (double)xv;
		y[j][k] = jobs;
	}
	fclose(stream);
	n = i - 1;
	xmax = xmin = x[0][0];
	ymax = ymin = y[0][0];
	/* find min and max of x and y */
	for(i=0; i<=tagh; i++)
	{
		for(j=0; j<=tagi[i]; j++)
		{
			if( x[i][j] > xmax ) xmax = x[i][j];
			if( x[i][j] < xmin ) xmin = x[i][j];
			if( y[i][j] > ymax ) ymax = y[i][j];
			if( y[i][j] < ymin ) ymin = y[i][j];
		}
	}
	yu = (ymax - ymin) / 40.0;
	xu = (xmax - xmin) / 90.0;
	/* find x,y value location on screen */
	for(i=0; i<=tagh; i++)
	{
		for(j=0; j<=tagi[i]; j++)
		{
			yd = (y[i][j] - ymin) / yu;
			xd = (x[i][j] - xmin) / xu;
			j1 = 41 - ((int)(yd + .5));
			j2 = (int)(xd + .5);
			yi[i][j] = j1;
			xi[i][j] = j2;
			j2 += 14;
			n = j2;
			k = 0;
			toggle = 1;
			while( screen[j1][j2] != ' ' )
			{
				if(toggle)
				{
					toggle = 0;
					j2 = n + (++k);
				}
				else
				{
					toggle = 1;
					j2 = n - k;
				}
			}
			screen[j1][j2] = tagc[i];
		}
	}
	/* generate label values for x and y axis */
	for(i=0; i<=10; i++)
	{
		yd = ( (double)i * 4.0 * yu) + ymin;
		sprintf(labtemp,"%8.3f",yd);
		for(j=0; j<=7; j++)
		{
			ylab[i][j] = labtemp[j];
		}
	}
	for(i=0; i<=9; i++)
	{
		xd = ((double)i * 10.0 * xu) + xmin;
		sprintf(labtemp,"%8.1f",xd);
		for(j=0; j<=7; j++)
		{
			xlab[i][j] = labtemp[j];
		}
	}
	/* place the axis label values on the screen */
	k = 41;
	for(i=0; i<=10; i++)
	{
		j1 = 0;
		for(j=4; j<=11; j++)
		{
			screen[k][j] = ylab[i][j1++];
		}
		k -= 4;
	}
	k = 43;
	j2 = 8;
	for(i=0; i<=9; i++)
	{
		j1 = j2;
		for(j=0; j<=7; j++)
		{
			screen[k][j1++] = xlab[i][j];
		}
		j2 += 10;
	}
	screen[21][2] = 'J';
	if(flag)
	{
		screen[44][48] = 'D';
		screen[44][49] = 'I';
		screen[44][50] = 'S';
		screen[44][51] = 'K';
	}
	else
		screen[44][48] = 'T';
	sprintf(s,"%s\0",var1);
	if(flag)
		sprintf(s1,"%s\0",var2);
	else
		sprintf(s1,"%s\0",var3);
	strcat(s,s1);
	scrcpy(45,s);
	sprintf(s,"                         CURVE SYMBOL    -    NUMBER OF TTY WRITE REQUEST PER JOB\0");
	scrcpy(46,s);
	sprintf(s,"             \0");
	for(i=2; i<=tagh; i++)
	{
		sprintf(s1,"CURVE %c %3d     \0",tagc[i],tag[i]);
		strcat(s,s1);
	}
	scrcpy(47,s);
	sprintf(s,"             I/O  ONLY --- CURVE T  TTY ONLY   CURVE D  DISK ONLY      MULTIUSER LEVEL EQUALS %2d\0",noproc);
	scrcpy(48,s);
	sprintf(s,"                    %s\0",title);
	scrcpy(49,s);
	printf("\f");
	for(i=0; i<=50; i++)
	{
		printf("%s\n",&screen[i][0]);
	}
	printf("\f");
	exit(0);
}

scrcpy(n,s)
	int n;
	char s[ ];
{
	register int i,j;

	i = n;
	j = 1;
	while( s[j] != '\0' )
	{
		screen[i][j] = s[j];
		j++;
	}
	return;
}
