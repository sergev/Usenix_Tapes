#
#include <stdio.h>
#include <math.h>
#include <h1500.h>
#define	TWO_DIV_PI	2/PI
#define IOSQR2PI	0.39894228040143268	/* 1/sqrt(2*pi) */
#define NX 250

/* T TEST WITH UNEQUAL OBSERVATION PATCH */
/* AND PAIRED DATA PATCH...S.K.22-APR-80 */
/* SOURCE: OLD FOCAL PROGRAM          */
/* ADDED ROUTINE TO CALC THE PROBABILITY FOR ONE TAILED TEST	*/
/* NOV 5, 1981.  S. KLYCE					*/

double n,n1,n2;
double x[NX],y[NX];
double  d,ds,q,r,m,m1,m2,s,s1,s2,t,u,a1,a2;

main()
{
	char ans[6];

	CLS;
	printf("		T TEST\n\n");
	printf("NEW VERSION....\n\tAddress complaints to Ms. Brouwer-Rutter !!\n\n");

	do strinp("DO YOU HAVE PAIRED OBSERVATIONS? (y or n) ", ans, 6);
	while (*ans != 'y' && *ans != 'n');
	if (*ans == 'y') paired();
	do strinp("DO YOU HAVE RAW DATA (r) OR MEANS  & S.E.M'S (m)? ",ans,6);
	while(*ans != 'r' && *ans != 'm');
	if (*ans == 'r') maria();
	do strinp("DO YOU HAVE EQUAL (e) OR UNEQUAL (u) OBSERVATIONS? ", ans,6);
	while(*ans != 'e' && *ans != 'u');
	if (*ans=='e') four1();
	else if (*ans=='u') five1();
}
maria()
{
	char ans[6];
	char anss[6];
	double tvalue(),p,matprint(),correct();
	int i;
	double asq, bsq, mean1, mean2, sp2, sp, dev1, dev2, se1, se2;

	numinp("NUMBER IN GROUP ONE =? ", &n1, 20);
	if (n1==0) exit();
	printf("GIVE %4.0f  VALUES OF FIRST GROUP\n", n1);
	for(i=0;i<n1;i++) {
		numinp("  =? ", &x[i], 20);
	}
	matprint(&n1,&x);
	do strinp("\nAny corrections? (y or n) ",ans,20);
	while(*ans != 'y' && *ans != 'n');
	*anss = *ans;
	if (*ans == 'y') correct(&n1,&x);
	if (*anss == 'y') matprint(&n1,&x);
	q=asq=0;
	for(i=0;i<n1;i++)
	{
		q =+ x[i];
		asq =+ x[i]*x[i];
	}
	dev1=sqrt((asq-(q*q)/n1)/(n1-1));
	se1=dev1/sqrt(n1);

	mean1=q/n1;
	printf("\n\n             n                        =%5.0f", n1);
	printf("\n\n             mean                     =%10.05f", mean1);
	printf("\n\n             standard deviation       =%10.05f", dev1);
	printf("\n\n             standard error           =%10.05f", se1);
	numinp("\n\n\n\nNUMBER IN GROUP TWO =? ", &n2, 20);
	printf("\nGIVE %4.0f VALUES OF SECOND GROUP\n", n2);
	for(i=0;i<n2;i++) numinp(" =? ", &y[i], 20);
	if (n2==0) exit();
	matprint(&n2,&y);
	do strinp("\nAny corrections? (y or n) ", ans, 20);
	while(*ans != 'y' && *ans != 'n');
	*anss = *ans;
	if (*ans == 'y') correct(&n2,&y);
	if (*anss == 'y') matprint(&n2,&y);
	r=bsq=0;
	for(i=0;i<n2;i++)
	{
		r =+ y[i];
		bsq =+ y[i]*y[i];
	}
	mean2=r/n2;
	dev2=sqrt((bsq-(r*r)/n2)/(n2-1));
	se2=dev2/sqrt(n2);
	printf("\n\n             n                      =%5.0f", n2);
	printf("\n\n             mean                   =%10.05f", mean2);
	printf("\n\n             standard deviation     =%10.05f", dev2);
        printf("\n\n             standard error         =%10.05f", se2);
	sp2=sp=t=p=0;
	sp=sqrt((asq+bsq-((q*q)/n1)-((r*r)/n2))/(n1+n2-2));
	t=fabs((mean1-mean2)/(sp*(sqrt(1/n1+1/n2))));
	printf("\n\n\n             t value                =%10.05f", t);
	printf("\n\n             degree(s) of freedom   =%5.0f", n1+n2-2);
	p=tvalue(t, (double)(n1+n2-2));
	printf("\n\n\n             P                      =%10.05f\n", p);
	seven();
	do strinp("\n\n\nMORE DATA (y or n) ", ans, 6);
	while(*ans != 'y' && *ans != 'n');
	if (*ans == 'y') maria();
	exit();
}

four1()
{
	char ans[6];
	double tvalue(),p;
	numinp("\nGROUP ONE MEAN =? ", &m1, 20);
	numinp( "GROUP ONE S.E.M. =? ", &s1, 20);
	numinp("GROUP TWO MEAN =? ", &m2, 20);
	numinp("GROUP TWO S.E.M. =? ", &s2, 20);
	numinp("\nNUMBER OF OBSERVATIONS =? ", &n, 20);

	if (n<=0) exit();
	t=(fabs(m1-m2))/sqrt(s1*s1+s2*s2);
	p=tvalue(t,n-1);
	printf("T VALUE =%8.04f\t",t);
	printf("P = %5.04f\n\n",p);
	seven();
	do strinp("MORE DATA (y or n) \n", ans, 6);
	while(*ans != 'y' && *ans != 'n');
	if (*ans == 'y') four1();
	exit();
}

five1()
{
	char ans[6];
	double tvalue(),p;
	numinp("\nGROUP ONE MEAN =? ", &m1, 20);
	numinp("GROUP ONE S.E.M. =? ", &s1, 20);
	numinp("NUMBER IN GROUP ONE =? ", &n1, 20);
	if (n1>NX) crash();
	numinp("\nGROUP TWO MEAN =? ", &m2, 20);
	numinp("GROUP TWO S.E.M. =? ", &s2, 20);
	numinp("NUMBER IN GROUP TWO =? ", &n2, 20);
	if (n1 <=0 || n2 <=0) exit();
	if (n2>NX) crash();
	a1=s1*s1*n1*(n1-1);
	a2=s2*s2*n2*(n2-1);
	t=fabs(m1-m2)/sqrt(((a1+a2)/((n1-1)+(n2-1)))*((1/n1)+(1/n2)));
	printf("\n\n  UNPAIRED UNEQUAL OBSERVATIONS:\n");
	printf(" T VALUE =%8.04f WITH %4.0f D.F.\t",t,n1+n2-2);
	p=tvalue(t,(double)(n1+n2-2));
	printf("P = %5.04f\n\n\n",p);
	seven();
	do strinp("MORE DATA (y or n) ", ans, 6);
	while(*ans != 'y' && *ans != 'n');
	if (*ans == 'y') five1();
	exit();
}

seven()
{
	int j;
	for (j=0;j<17;j++)
	{
		printf(" *");
		printf(" *");
	}
	printf("\n\n");
}

crash()
{
	printf("\n\n\nSorry friend, the number of variables exceeds anticipated dimensions.\nSee local guru for help!\n\n\n");
	exit();
}

paired()
{
	char ans[6];
	char anss[6];
	double tvalue(),p;
	int i;
	double sd2,s1s2,dsq,esq,meanx,meany,dv1,dv2,stde1,stde2,n,sum;
	double x[NX],y[NX];
	double tval;
	numinp("\nHOW MANY DATA PAIRS =? ", &n, 20);
	if (n==0) exit();
	printf("\nPLEASE ENTER %3.0f PAIRS\n",n);
	sum=2*n;
	for(i=0;i<sum;i++)
	{
		if ((i+1)%2)
			printf("\nPair %3d:  ", i/2+1);
		numinp(" =? ", &x[i], 20);
	}
	matprint(&sum,&x);
	do strinp("\nAny corrections? (y or n)" , ans, 20);
	while(*ans != 'y' && *ans != 'n');
	*anss = *ans;
	if (*ans == 'y') correct(&sum,&x);
	if (*anss == 'y') matprint(&sum,&x);
	s1=s2=dsq=esq=0;
	for(i=0;i<sum;i+=2)
	{
		s1 =+ x[i];
		s2 =+ x[i+1];

		dsq =+ x[i]*x[i];
		esq =+ x[i+1]*x[i+1];
	}
	meanx=s1/n;
	meany=s2/n;

	dv1=sqrt((dsq-(s1*s1)/n)/(n-1));
	dv2=sqrt((esq-(s2*s2)/n)/(n-1));

	stde1=dv1/sqrt(n);
	stde2=dv2/sqrt(n);

	printf("\n\n            GROUP I");
	printf("\n\n              n                        =%5.0f", n);
	printf("\n\n              mean                     =%10.05f",meanx);
	printf("\n\n              standard deviation       =%10.05f",dv1);
	printf("\n\n              standard error           =%10.05f",stde1);
	printf("\n\n              GROUP II");
	printf("\n\n              n                        =%5.0f",n);
	printf("\n\n              mean                     =%10.05f",meany);
        printf("\n\n              standard deviation       =%10.05f",dv2);
	printf("\n\n              standard error           =%10.05f",stde2);
	t=0;
	sd2=0;
	for(i=0;i<sum;i+=2)
	{
		t =+ x[i]-x[i+1];
		sd2 =+ (x[i]-x[i+1])*(x[i]-x[i+1]);
	}
	printf("\n\nT FOR PAIRED OBSERVATIONS =");
	if (sd2-t*t/n!=0)
		tval=fabs((t/n)/(sqrt((sd2-t*t/n)/(n*(n-1)))));
	else tval=0;
	printf("%8.04f",tval);
	printf(" FOR %3.0f D.F.\t",n-1);
	p=tvalue(tval,(double)(n-1));
	printf("P = %5.04f\n\n\n",p);
	seven();
	do strinp("\nMORE DATA (y or n) ", ans, 6);
	while(*ans != 'y' && *ans != 'n');
	if (*ans == 'y') paired();
	exit();
}


/*
	This routine was translated from Basic 11/03/81.
	Source & description: BASIC-Pack Statistics Programs for Small
			      Computers by D. Van Tassel. Prentice-Hall

						S.Klyce
*/

double tvalue(t,n)
double t;
double n;
{
	double erf();
	double t9,n9,z,y,p,a,t2;
	double b;
	int j;
	if (t <= 0 || n < 1) return(-1);
	else
	{
		t9=t;
		n9=n;
		z=1;
		t *= t;
		y = t/n;
		b = 1 + y;
		if ( n > (int)n ) goto l1510;
		if ( n < 20 ) goto l1490;
		if ( t < n ) goto l1510;
l1490:
		if ( n > 200 ) goto l1510;
		goto l1600;
l1510:
		if (y > .1e-5) y = log(b);
		a = n - 0.5;
		b = 48 * a * a;
		y *= a;
		y=(((((-0.4*y-3.3)*y-24)*y-85.5)/(0.8*y*y+100+b)+y+3)
			/b+1)*sqrt(y);
		t2=2*erf(-y);
		goto l1930;
l1600:
		if (n >= 20) goto l1780;
		if (t>=4) goto l1780;
		y = sqrt(y);
		a=y;
		if (n==1) goto l1670;
		a=0;
l1670:
		n -= 2;
		if (n<=1) goto l1710;
		if (b && n) a=(n-1)/(b*n)*a+y;
		goto l1670;
l1710:
		if (n==0) goto l1740;
		if (b) a=(atan(y)+a/b)*TWO_DIV_PI;
		goto l1760;
l1740:
		if (b) a /= sqrt(b);
l1760:
		t2=z-a;
		goto l1930;
l1780:
		a=sqrt(b);
		y=a*n;
		j=2;
l1820:
		if (a==z) goto l1880;
		z=a;
		if (b && j) y *= (j-1)/(b*j);
		if (n && j) a += y/(n+j);
		j += 2;
		goto l1820;
l1880:
		n += 2;
		z=y=0;
		a = -a;
		goto l1670;
l1930:
		return(t2);
	}
}
	
double erf(xarg)
double xarg;
{
	int d;
	double z2,z4,a9,p,p9,x;
	d=sgn(xarg);
	xarg=fabs(xarg);
	z2=xarg*xarg;
	a9=z4=xarg*IOSQR2PI;
	p=1;x=0;
l2210:
	x += 1;
	p = -p*z2/2/x;
	p9=p/(2*x+1)*z4;
	if (fabs(p9)<0.1e-16) goto l2270;
	a9 += p9;
	goto l2210;
l2270:
	if (d==1) goto l2300;
	a9 = 0.5 - a9;
	goto l2310;
l2300:
	a9 += 0.5;
l2310:
	return(a9);
}

sgn(z)
int z;
{
	if (!z) return(0);
	return(z > 0 ? 1 : -1);
}


/*****************************************************/
/***    NUMINP.C      Feb 18, 1982       S. KLYCE  ***/
/*****************************************************/

/* #include <stdio.h> */

numinp(s,num,lim)
char *s;
double *num; /* double *num for double precision */
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
	}
	*num = atof(svar); /* remove (float) for double precision. */
	return(done);
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


double matprint(n,array)
double *n;
double *array;
{
	int ko;
	int i;
	ko=0;
	printf("\n");
	while (ko < *n)
	{
		for (i=0;i<4;i++)
		{
			if (i+ko < *n)
			{
				printf(" val[%3d]= %6.02f",ko+i+1,array[ko+i]);
			}
		}
		ko =+ 4;
		printf("\n");
	}
	printf("\n\n");
}

double correct(n,array)
double *n;
double *array;
{
	char ans[6];
	int i;
	float ix;
	float xl;
	do numinp("\nEnter index no. of wrong entry =? ",&ix,7);
	while (ix <= 0 || ix > *n);
	i=ix;
	printf("\nval[%3d]", i);
	numinp(" =? ", &xl, 20);
	array[i-1]=xl;
	do strinp("More ? (y or n) ",ans,10);
	while(*ans != 'y' && *ans != 'n');
	if (*ans == 'y') correct(n,array);
}
