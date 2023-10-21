/* This program is hereby given into the hands of the public, and is 
 * considered to be public domain.  You may use it for whatever purposes
 * you wish, including but not limited to: resale, personal use, teaching,
 * incorporating into other programs, fun at parties.  The author, 
 * regrettably, does not warrant it for any purpose whatsoever, so you're
 * on your own.
 */

#include <stdio.h>
#include <sgtty.h>
#include <math.h>

struct sgttyb otsgb;
char mnths[12][4] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

main()
{
	char choice;
	
	iniline();
	clear_init();
	resetline();

	while(1)
	{
		clear();
		printf("\tFINANCIAL HELPER\n\n");
		printf("0) Calculate Rate of Return\n");
		printf("1) Interest Rate on Loan\n");
		printf("2) Future Value of monthly deposits\n");
		printf("3) Nominal & Effective Interest Rates\n");
		printf("4) Compound Interest Rate\n");
		printf("5) Simple Interest Rate\n");
		printf("6) Future Value of Present Sum\n");
		printf("7) Present Value of Future Sum\n");
		printf("8) Amortization Schedule\n");
		printf("9) Done\n");
		printf("\nPlease choose one of the above ");
		fflush(stdout);
		iniline();
		choice = getchar();
		resetline();
		clear();
		switch (choice)
		{
			case '0': ROR(); break;
			case '1': LOANINT(); break;
			case '2': FVMP(); break;
			case '3': NEIR(); break;
			case '4': CIR(); break;
			case '5': SIR(); break;
			case '6': FVPS(); break;
			case '7': PVFS(); break;
			case '8': AMORT(); break;
			case '9': printf("\n");exit(0);
		}
	}
}

ROR()
{
	double presval,futval,earnings[100],temp0,temp1,temp2,temp3;
	double finput();
	int i,j, numpers, numyears, iinput();
	char a;

	presval = finput("\tPurchase Price of investment ");
	futval  = finput("\tFinal Sales price ");
	numpers = iinput("\tTotal number of periods ");
	numyears= iinput("\tNumber of periods in each year ");
	for(i=0;i<numpers;i++)
	{
		printf("\tEarnings for period %d",i+1);
		earnings[i] = finput(" ");
	}
	temp0 = 0.15 / numyears;
	while(1)
	{
		temp1 = futval / pow((float)(1+ temp0), (float)numpers);
		temp3 = temp2 = 0.0;
		for (i=0;i<numpers;i++)
		{
			temp3 = earnings[i] / pow((float)(1+temp0),(float)i);
			temp2+=temp3;
		}
		temp3 = temp0 * (temp1 + temp2) / presval;
		if(temp0 - temp3 < 0.000001 && temp0 - temp3 > -0.000001)
			break;
		else
			temp0 = temp3;
	}
	printf("\nRate of Return = %11.2f",temp0*numyears*100);
	myret();
}

double
finput(string)
char *string;
{
	char *temp, *buf, *malloc();
	int i;

	printf("%s",string);
	fflush(stdout);
	buf = temp = malloc(20);
	for(i=0; i<20;i++)
	{
		*buf = getchar();
		if (*buf == '\n')
			break;
		else
			buf++;
	}
	if(i == 20)
		buf[-1] = '\0';
	else
		*buf = '\0';
	return(atof(temp));
}

iinput(string)
char *string;
{
	char *temp, *buf, *malloc();
	int i;

	printf("%s",string);
	buf = temp = malloc(20);
	for(i=0; i<20 && buf[-1] != '\n';i++)
		*buf++ = getchar();
	buf[-1] = '\0';
	return(atoi(temp));
}

LOANINT()
{
	double l, pv, p, l1;
	int ny, n;
	
	pv = finput("\tPresent value of loan     ");
	ny = iinput("\tNumber of terms per year  ");
	n  = iinput("\tNumber of periods in loan ");
	p  = finput("\tAmount of each payment    ");
	l = 0.008;
	while(1)
	{
		l1 = p / pv * pow((float)(1+l),(float)(n-1)) / pow((float)(1+l),(float)n);
		if (l-l1 < 0.000001 && l-l1 > -0.000001)
			break;
		else
			l = l1;
	}
	l = l1 * ny * 100;
	printf("\tInterest rate on this loan is %11.4f %% \n",l);
	myret();
}
FVMP()
{
	double rd, ir, t, i, fv;
	int py, m, y;
	
	rd = finput("\tAmount of regular deposit   ");
	py = iinput("\tNumber of deposits per year ");
	m  = iinput("\tTotal number of months      ");
	y = m / 12;
	ir = finput("\tNominal interest rate       ");
	i = ir / py / 100.0;
	t = pow((float)(1+i),(float)(py*y));
	t--;
	t /= i;
	fv = rd * t;
	fprintf(stdout,"\tFuture value = $%11.2f \n",fv);
	fflush(stdout);
	myret();
}
NEIR()
{
	double fv, pv, nr, er;
	int np, py;

	fv = finput("\tFuture value              $");
	pv = finput("\tPresent value             $");
	np = iinput("\tTotal number of periods    ");
	py = iinput("\tNumber of periods per year ");
	nr = (py * (pow((float)(fv / pv), (float)(1 / (float)np))) - py) *100.0;
	er = (pow((float)(fv / pv) , (float)(py / (float)np)) - 1) * 100.0;
	fprintf(stdout,"\tNominal rate = %8.4f %%\n",nr);
	fprintf(stdout,"\tEffective rate = %8.4f %%\n",er);
	fflush(stdout);
	myret();
}
CIR()
{
	double fv, pv, i, pd;
	int y, m, d, py;
	
	fv = finput("\tFuture value              $");
	pv = finput("\tPresent value             $");
	py = iinput("\tNumber of periods per year ");
	y  = iinput("\tNumber of years            ");
	m  = iinput("\tNumber of months           ");
	d  = iinput("\tNumber of days             ");
	pd = y*py+m*py/12+d*py/365;
	i = (pow((float)(fv / pv), (float)(1.0/pd)) - 1) * 100 * py;
	fprintf(stdout,"\tAnnual interest rate = %8.4f %%\n",i);
	fflush(stdout);
	myret();
}
SIR()
{

	fprintf(stdout,"Not Currently available\n");
	fflush(stdout);
	myret();
}
FVPS()
{
	double p, r, i, t, a, s, f;
	int m, n, k;

	p = finput("\tPresent sum                  $");
	r = finput("\tAnnual interest rate          ");
	m = iinput("\tNumber of periods per year    ");
	n = iinput("\tNumber of periods to maturity ");
	i = r / (float)m;
	i /= 100.0;
	t = i + 1.0;
	a = t;
	if (n != 1)
		for(k = 1; k <= n-1; k++)
		{
			s = a * t;
			a = s;
		}
	f = p * a;
	fprintf(stdout,"\tFuture value = $%11.2f \n",f);
	fflush(stdout);
	myret();
}
PVFS()
{
	double f, r, i, t, a, s, p;
	int m, n, x;

	f = finput("\tFuture sum                $");
	r = finput("\tAnnual interest rate       ");
	m = iinput("\tNumber of periods per year ");
	n = iinput("\tTotal number of periods    ");
	i = r / (float)m;
	i /= 100.0;
	t = 1.0 + i;
	a = t;
	if(n != 1)
		for(x = 1; x <= n-1; x++);
		{
			s = a * t;
			a = s;
		}
	p = f / a;
	fprintf(stdout,"\tPresent value = $%11.2f\n",p);
	fflush(stdout);
	myret();
}
AMORT()
{
	double b,k,t,p,l,r,m,ay,at,a;
	double finput();
	int yr, c, L, z;
	char chr, prtflg;
	FILE *printer, *fopen(); 

	c = yr = 0;
	p = finput("\tPrincipal Amount ");
	L = iinput("\tNumber of months ");
	r = finput("\tAnnual interest rate (ie 15.0 or 8.8) ");
	m = finput("\tPayment, if known ");
	while(c < 1 || c > 12)
		c = iinput("\tPayment starts on month number ");
	while(yr < 1 || yr > 3000)
		yr = iinput("\tYear payments start ");
	c--;
	yr--;
	fprintf(stdout,"Print schedule on printer? (y,n)");
	fflush(stdout);
	iniline();
	prtflg = getchar();
	resetline();
	if(prtflg == 'Y' || prtflg == 'y')
	{
		printer = fopen("/dev/lp","w");
		fflush(printer);
	}
	else
		printer = stdout;
	l = r / 1200.0;
	t = 1.00 - (1.00 / pow((float)(1+l), (float)L));
	k = p;
	if (m == 0.0)
		m = p * l / t;
	amorthead(printer);
	at = ay = 0.0;
	for (z = 0; z < L; z++)
	{
		if (c >= 12)
		{
			yr++;
			fprintf(printer,"\nYear of loan: %d\n",yr);
			fprintf(printer,"\t$%11.2f for %d months at %11.2f %% \n",k,L,r);
			at = at + ay;
			fprintf(printer,"\tTotal interest paid during year --> $%11.2f\n\n",ay);
			fflush(printer);
			if (prtflg != 'Y' && prtflg != 'y')
			{
				printf("\n\tPress return to continue, 'q' to end ");
				fflush(stdout);
				iniline();
				chr = getchar();
				resetline();
				if (chr == 'q')
					return(0);
			}
			ay = 0.0;
			c = 0;
			amorthead(printer);
		}
		a = p * l;
		b = m - a;
		p = p - b;
		ay = ay + a;
		fprintf(printer,"%s      %03d %11.2f %11.2f %11.2f %11.2f\n"
			,mnths[c],z+1,p,m,b,a);
		fflush(printer);
		if (p > 0.0)
			c++;
		else
			break;
	}
	fprintf(printer,"\n\nYear of loan: %d\n",yr+1);
	fprintf(printer,"\t$%11.2f for %d months at %11.2f %%\n",k,L,r);
	fprintf(printer,"\tTotal interest paid during year --> $%11.2f\n",ay);
	at = at + ay;
	fprintf(printer,"\tTotal interest paid during loan --> $%11.2f\n",at);
	fflush(printer);
	myret();
	if(prtflg == 'Y' || prtflg == 'y')
		fclose(printer);
}

amorthead(printer)
FILE *printer;
{
	if(printer != stderr)
		fprintf(printer,""); /* A bleedin' form feed.... */
	else
		clear();
	fprintf(printer,"      PAYMENT  REMAINING     MONTHLY    PRINCIPAL   INTEREST\n");
	fprintf(printer,"MNTH  NUMBER   PRINCIPAL     PAYMENT     PAYMENT     PAYMENT\n");
	fflush(printer);
}

iniline()
{
	struct sgttyb ntsgb;

	if(isatty(fileno(stdin)))
	{
		ioctl(fileno(stdin), TIOCGETP, &otsgb);
		ntsgb = otsgb;
		ntsgb.sg_flags |= CBREAK;
		ioctl(fileno(stdin), TIOCSETP, &ntsgb);
	}
}

resetline()
{
	if(isatty(fileno(stdin)))
	{
		ioctl(fileno(stdin), TIOCSETP, &otsgb);
	}
}
myret()
{
	fprintf(stdout,"\n\tPress RETURN to continue");
	fflush(stdout);
	iniline();
	getchar();
	resetline();
}

char PC;
short ospeed;
int numlines;
char *clr;
char clbuf[20];	/* contains 'cl' sequence */

clear_init()
{
	char *tgetstr();
	char *getenv();
	char *buffer;	/* termcap info */
	char *pbp;	/* contains pad character */
	char *malloc();
	char *cbp = clbuf; /* contains 'cl' sequence */

	pbp = malloc(20);	/* we're screwed anyway so don't bother */
	buffer = malloc(1024);	/* to look at what was returned? */
        ospeed = otsgb.sg_ospeed; /* was set in iniline routine */
        tgetent(buffer, getenv("TERM")); /* get entry from /etc/termcap */
        clr = tgetstr("pc", &pbp);	/* get pad character */
	PC = (*clr ? *clr : 0);		/* NULL or actual character */
        clr = tgetstr("cl", &cbp);	/* get clear sequence */
	numlines = tgetnum("li");	/* get number of lines on screen */
	free(pbp);
	free(buffer);
}

#undef  putchar
int putchar();

clear()
{
        if (clr)
                tputs(clr, numlines, putchar);
	printf("\n");	/* What the heck */
}
