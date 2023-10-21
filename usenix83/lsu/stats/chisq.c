#
#include <stdio.h>
#include <math.h>
/* CHI-SQUARE FOR 2X2 CONTINGENCY TABLE
31-MAR-80................S. KLYCE    */
/* TRANSLATED WITHOUT APOLOGY TO C FROM FOCAL */
long na,nb,nc,nd,t1,t2,t3,t4,t,nf;
int i;
double x2,p,s1;
main()
{
	start:
	printf("\n\n                     CHI-SQUARE TEST: 2X2");
	printf("\n\nNo. of control experiments that showed an effect =? ");
	scanf("%ld",&na);
	printf("Out of how many control experiments =? ");
	scanf("%ld",&t1);
	printf("No. of treated experiments that showed an effect =?");
	scanf("%ld",&nb);
	printf("Out of how many treated experiments =?");
	scanf("%ld",&t2);
	printf("\n\n\n                             CONTINGENCY TABLE");
	nc=t1-na;
	nd=t2-nb;
	t3=nc+nd;
	t4=na+nb;
	t=t1+t2;
	if (na<5 || nb<5 || nc<5 || nd<5) rule5();
	else normal();
	goto start;
}

normal()
{
	x2 = fabs((double)(na*nd-nb*nc))-t/2.;
	x2 = x2*x2*t/(t1*t2*t3*t4);
	tablepr();
	printf("\n\n         CHI-SQUARE=%12.03f",x2);
	if (x2>5.41)
	{
		printf("\n          P < 0.01; significant at the 0.01 level\n");
		termpr();
	}
	else if (x2>2.71)
	{
		printf("\n          0.05>P>0.01; significant at the 0.05 level\n");
		termpr();
	}
	else 
	{
		printf("\n          P > 0.05; not significant\n");
		termpr();
	}
}

rule5()
{
	printf("\n\nRule of 5 employed...\n");
	nf=t1;
	four2();
	p=s1;
	nf=t2;
	four2();
	four3();
	nf=t3;
	four2();
	four3();
	nf=t4;
	four2();
	four3();
	nf=t;
	four2();
	four4();
	nf=na;
	four2();
	four4();
	nf=nb;
	four2();
	four4();
	nf=nc;
	four2();
	four4();
	nf=nd;
	four2();
	four4();
	tablepr();
	printf("\n\n       P = %7.04f",p);
	termpr();
}
four2()
{
	s1=1;
	if (nf < 0)
	{
		printf("\n\nYou goofed.  Try again.\n\n");
		exit();
	}
	else if (nf == 0) nf=1;
	else for (i=1;i<=nf;i++) s1 = s1*i;
}

four3()
{
	p =* s1;
}

four4()
{
	if (s1==0)
	{
		p=0;
		printf("\nSorry...number too large to handle\n");
		exit();
	}
	else p =/ s1;
}

termpr()
{
	 printf("\n                ");
	for (i=0;i<12;i++) printf("<> ");
}

tablepr()
{
	printf("\n                ");
	for (i=0;i<41;i++) printf("+");
	printf("\n                +  Control  +     Treated     +  Total  +\n");
	for (i=0;i<57;i++) printf("+");
	printf("\n+ Effect        + %5ld",na);
	printf("     +     %5ld",nb);
	printf("       +  %5ld  +\n",t4);
	for (i=0;i<57;i++) printf("+");
	printf("\n+ No Effect     + %5ld     +     %5ld       +  %5ld  +\n",nc,nd,t3);
	for (i=0;i<57;i++) printf("+");
	printf("\n+ Total         + %5ld     +     %5ld       +  %5ld  +\n",t1,t2,t);
	for (i=0;i<57;i++) printf("+");
	printf("\n\n");
}
