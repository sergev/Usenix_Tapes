#include <stdio.h>

static int noproc1,noproc2;
static int notty1,notty2,nodkr1,nodkr2,toggle;
static char screen[45][108],c[5],s1[101],s2[101];
static double xmax,xmin,ymax,ymin;
static int xi[10][50],yi[10][50];
static double x[10][50],y[10][50];
static int tagv,xv;
static char tagc[11] =
{"ABCEFGHIJK"};
static int flag = 1;
static int tagi[10] =
{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
static int tagh = -1;
static int tag[10] =
{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
static double xu,yu,xd,yd;
static float jobs1,jobs2;
FILE * stream1;
FILE * stream2;
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
{"           R = RATIO OF TWO SYSTEMS THROUGHPUT"};
static char var2[ ] =
{"           D = DISK READ + WRITE REQUEST PER JOB"};
static char var3[ ] =
{"           T = TTY WRITE REQUEST PER JOB"};
	static char title1[100],title2[100];

main(argc,argv)
	int argc;
	char * argv[ ];
{
	register int i,n,j,j1,j2,k;

	if( argc != 3 )
	{
		printf("incorect number of arguments\n");
		exit(1);
	}
	if( (stream1 = fopen(argv[1],"r")) == NULL )
	{
		printf("unable to open file %s\n",argv[1]);
		exit(2);
	}
	if( (stream2 = fopen(argv[2],"r")) == NULL )
	{
		printf("unable to open file %s\n",argv[2]);
		exit(2);
	}
	/* read in titles */
	if( fgets(title1,100,stream1) == NULL )
	{
		printf("The file %s is empty\n",argv[1]);
		exit(1);
	}
	if( fgets(title2,100,stream2) == NULL )
	{
		printf("The file %s is empty\n",argv[2]);
		exit(1);
	}
	if( fgets(s1,100,stream1) == NULL )
	{
		printf("The file %s is empty\n",argv[1]);
		exit(1);
	}
	sscanf(s1,"%d",&noproc1);
	if( fgets(s2,100,stream2) == NULL )
	{
		printf("The file %s is empty\n",argv[2]);
		exit(1);
	}
	sscanf(s2,"%d",&noproc2);
	n = strlen(title1);
	title1[n-1] = '\0';
	n = strlen(title2);
	title2[n-1] = '\0';
	/* clear screen */
	for(i=0; i<=44; i++)
	{
		for(j=0; j<=107; j++)
		{
			if( j == 107 )
				screen[i][j] = '\0';
			else
				screen[i][j] = ' ';
		}
	}
	/* put in x and y axis */
	j = 11;
	for(i=0; i<=40; i++)
	{
		screen[i][j] = yaxis[i];
	}
	i = 41;
	k = 0;
	j = 12;
	while( j <= 102 )
	{
		screen[i][j] = xaxis[k++];
		j++;
	}
	/* read in y and x values */
	i = 0;
	while( (fgets(s1,100,stream1) != NULL) &&
		(fgets(s2,100,stream2) != NULL) )
	{
		sscanf(s1,"%d  %d  %f",&notty1,&nodkr1,&jobs1);
		sscanf(s2,"%d  %d  %f",&notty2,&nodkr2,&jobs2);
		if( (notty1 != notty2) || (nodkr1 != nodkr2) )
		{
			printf("benchmark data files are out of sequence\n");
			exit(1);
		}
		if(flag)
		{
			tagv = notty1;
			xv = nodkr1;
		}
		else
		{
			tagv = nodkr1;
			xv = notty1;
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
		y[j][k] = jobs1 / jobs2;
	}
	fclose(stream1);
	fclose(stream2);
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
			j1 = 40 - ((int)(yd + .5));
			j2 = (int)(xd + .5);
			yi[i][j] = j1;
			xi[i][j] = j2;
			j2 += 12;
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
	k = 40;
	for(i=0; i<=10; i++)
	{
		j1 = 0;
		for(j=3; j<=10; j++)
		{
			screen[k][j] = ylab[i][j1++];
		}
		k -= 4;
	}
	k = 42;
	j2 = 6;
	for(i=0; i<=9; i++)
	{
		j1 = j2;
		for(j=0; j<=7; j++)
		{
			screen[k][j1++] = xlab[i][j];
		}
		j2 += 10;
	}
	screen[20][2] = 'R';
	if(flag)
		screen[43][46] = 'D';
	else
		screen[43][46] = 'T';
	printf("\f");
	for(i=0; i<=43; i++)
	{
		printf("%s\n",&screen[i][0]);
	}
	printf("%s",var1);
	if(flag)
		printf("%s\n",var2);
	else
		printf("%s\n",var3);
	printf("                       CURVE SYMBOL    -    NUMBER OF TTY WRITE REQUEST PER JOB\n");
	printf("           ");
	for(i=0; i<=tagh; i++)
	{
		printf("CURVE %c %3d     ",tagc[i],tag[i]);
	}
	printf("\n           MULTIUSER LEVELS EQUAL %2d %2d\n",noproc1,noproc2);
	printf("           *** THE THROUGHPUT RATIO OF %s  VRS  %s ***\n",
		title1,title2);
	exit(0);
}
