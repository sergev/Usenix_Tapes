#
#include <stdio.h>
#include <math.h>
/* Mean.c  -  mean, std err, std dev.
	S. Klyce - LSU/NO	*/
char *ans;
char *anss;
float nx;
int n;
int i;
int runflag 1;
float p;
float sq;
float se;
float sx;
float sd;
float x[1000];
main()
{
	printf("\n\n\n\tMEAN, STD DEVIATION & STD ERROR");
	printf("\n\nEnter data.  When finished type 'done' <-- NOTE NEW TERMINATOR!\n\n");
	for (i=0;i<20;i++) printf(" ");
	for (i=0;i<8;i++) printf("<> ");
	printf("\n\n");
	while (runflag)
	{
		n=0;
		while (!numinp("x =? ",&x[n],20)) n++;
		matprint();
		loop:
		strinp("\nAny corrections? (y or n) ",ans,20);
		*anss = *ans;
		if (*ans == 'y') correct();
		else if (*ans != 'n') goto loop;
		if (*anss=='y') matprint();
		errcalc();
		strinp("\n\nMore data? (y or n) ",ans,20);
		if (*ans=='n') runflag=0;
	}
}

errcalc()
{
	sx=0;
	for (i=0;i<n;i++)
	
	{
		sx =+ x[i];
		p = sx/n;
	}
	sq=0;
	for (i=0;i<n;i++)
	{
		sq =+ x[i]*x[i];
	}
	sd=sqrt((sq-(sx*sx)/n)/(n-1));
	nx=n;
	se=sd/sqrt(nx);
	printf("\n\n             n                     =%10d",n);
	printf("\n\n             mean                  =%10.05f",p);
	printf("\n\n             standard deviation    =%10.05f",sd);
	printf("\n\n             standard error        =%10.05f",se);

}

matprint()
{
	int ko;
	ko=0;
	printf("\n");
	while (ko < n)
	{
		for (i=0;i<5;i++)
		{
			if (i+ko < n)
			{
				printf(" x[%3d]= %6.02f",ko+1+i,x[ko+i]);
			}
		}
		ko =+ 5;
		printf("\n");
	}
	printf("\n\n");
}

correct()
{
	int i;
	float ix;
	float xl;
	loopc:
	numinp("\nEnter index no. of wrong entry =? ",&ix,7);
	i=ix;
	numinp("\nx[]=? ",&xl,20);
	x[i-1]=xl;
	loopd:
	strinp("More ? (y or n) ",ans,10);
	if (*ans == 'y') goto loopc;
	else if (*ans != 'n')goto loopd;
	matprint();
}
/*******************************************************/
/***   STRINP.C     Feb 18, 1982    S. KLYCE         ***/
/*******************************************************/

/* #include <stdio.h> */
#define MINASCII	'\037'   /**  These two lines           **/
#define MAXASCII	'\176'   /** terminal dependent.        **/

strinp(s,svar,lim)
char *s, *svar;
int lim;
{
	int c,i;
	int error = 1;
	while (error) {
		printf("%s",s);
		error=i=0;
		while (--lim > 0 && (c=getchar()) != EOF && c != '\n')
		{
			if ((c < MINASCII || c > MAXASCII) && c != '\n') error=1;
			svar[i++] = c;
		}
		if (c == '\n')
			svar[i++] = c;
		svar[i] = '\0';
	}
}
/*****************************************************/
/***    NUMINP.C      Feb 18, 1982       S. KLYCE  ***/
/*****************************************************/

/* #include <stdio.h> */

numinp(s,num,lim)
char *s;
float *num; /* double *num for double precision */
int lim;
{
	extern double atof();
	int c,i;
	int done=0;
	char *svar='\0';
	int error = 1;
	while (error) {
		printf("%s",s);
		error=i=0;
		*svar='\0';
		while (--lim > 0 && (c=getchar()) != EOF && c != '\n') {
			if ((c < '0' || c > '9') && c != 'e' && c != '-' && c != '.') error=1;
			svar[i++]=c;
		}
		svar[i] = '\0';
		if (*svar == '\0') error=1;
		if (strcmp(svar,"done")==0) {
			error=0;
			done=1;
		}
		if (error) printf("ERROR\n");
	}
	*num = (float)atof(svar); /* remove (float) for double precision. */
	return(done);
}
