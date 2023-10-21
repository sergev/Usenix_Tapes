
/****************************************************************************
*	C version of "roi" awk script last posted to the net Jan 86.
*	This is pretty much a straight conversion, I took most of the
*	printf's from the awk version.  Since the "sell" transaction
*	types didn't seem to work in the awk version, I chose not to 
*	handle them at all.  This code has been ported to the Amiga 
*	using the native Lattice C compiler.  It was thrown
*	together in the wee hours of the morning and could use alot
*	more error checking.  
*
*	I don't plan to do much more to this but I would like modifications
*	sent to me.
*	
*	Disclaimer:
*	No warranties expressed or implied.  I am placing this in the public
*	domain and neither I nor my employer assume responsibility
*	in any way for it. 
*	What this means is if you lose 10^9 dollars because this program screws
*	up, thats your problem.  You shouldn't be trusting 10^9 dollars
*	to the likes of this code.
*
*	George Jenkins, COSI Texas, Inc., 4412 Spicewood Springs #801, Austin TX
*	78759 USA
*
*	uucp: {ihnp4,seismo,ctvax}!ut-sally!cositex!gj
*	at&t: (512) 345-2780
****************************************************************************/

/****************************************************************************
*	Input file format (taken from awk script):
*
*		the first record of the input must indicate today's date
*		in the form
*
*			date	mm	dd	yy
*
*		where date is literally 'date' and mm, dd, and yy are as you 
*		would expect.
*
*		the second record is of the form
*
*			price	xxx.xx
*
*		where price is literally 'price' and xxx.xx is today's market
*		value of the stock.
*
*		NOTE: there can be many price and date records, only
*		the last ones are used.  This is a good way to keep
*		a price record.
*
*		a purchase transaction has the form
*
*			buy  mm  dd  yy  bbb.bb  ppp.pp  nnn.nn
*
*		where buy is literally 'buy', mm, dd, and yy are the date of
*		the purchase transaction, bbb.bb is the amount of dollars
*		invested, ppp.pp is the per share price, and nnn.nn is the
*		number of shares purchased.
*
*		a stock dividend transaction has the form
*
*			div  mm  dd  yy  ppp.pp
*
*		where div is literally 'div' and ppp.pp is the percentage of
*		dividend awarded.
*
*		A reinvested cash distribution has the form
*
*			rein  mm  dd  yy  bbb.bb  ppp.pp  nnn.nn
*
*		where rein is literally "rein' and the rest are treated
*		the same as a buy transaction.  Any reinvested gain
*		must also be recorded in a buy record for basis to be
*		correctly calculated.
*
*		A cash distribution (that is not reinvested) has the form
*
*			cash  mm  dd  yy  bbb.bb
*
*		where cash is literally 'cash' and bbb.bb is the amount of
*		cash received.
****************************************************************************/

#include <stdio.h>
#include <math.h>
#include <ctype.h>

#define MAXBUYS	100	/* maximum number of "buy" transactions */

struct date {
	int mm;
	int dd;
	int yy;
};

struct buy {
	struct date when;
	float basis, pershare, nshares, currentvalue;
	int heldmonths;
} buy_trans[MAXBUYS];

int nbuy_trans;

struct date curdate;
int curmonth;		/* internal form of date */

float currentprice;
float reinshares, reinbasis;
float cumcash;

char *get_date();

main()
{

	char buf[128];

	printf ("Stock Investment Program ");
	printf ("\n");
	while(gets(buf) != NULL)
		process_trans(buf);
	summary();
}

int proc_buy(), proc_div(), proc_cash(), proc_rein(), proc_date(), 
	proc_price();

struct func_tab {
	char *trans_type;
	int (*trans_func)();
} lookup[] = {
	"buy",		proc_buy,
	"div",		proc_div,
	"cash",		proc_cash,
	"rein",		proc_rein,
	"date",		proc_date,
	"price",	proc_price,
	0
};

process_trans(buf)
char *buf;
{

	char type[20];
	register struct func_tab *t;

	if(sscanf(buf, "%s", type) == 1){
		buf += strlen(type) + 1;
		for(t = lookup; t->trans_type; t++)
			if(!strcmp(type, t->trans_type)){
				(*t->trans_func)(buf);
				break;
			}
		if(!t->trans_type)
			printf("unknown type - %s\n", type);
	}
}

float cumbasis, cumshares;

summary()
{
	register int i;
	register struct buy *b;

	for(i = 0, b = buy_trans; i < nbuy_trans; i++, b++){
		if((i % 40) == 0)
			print_heading();
		print_buy(b);
		printf("\n");
	}
	printf("\n");
	printf("\n");
	printf ("distribution dollars -  %12.2f", reinbasis);
	printf ("\n");
	printf ("distribution shares  -  %14.4f", reinshares);
	printf ("\n");
	printf ("\n");
	printf ("total investment dollars -  %8.2f", cumbasis - reinbasis);
	printf ("\n");
	printf ("total investment shares  -  %10.4f", cumshares - reinshares); 
	printf ("\n");
	printf ("\n");
	printf ("total value - %22.2f", currentprice * cumshares);
	printf ("\n");
	printf ("accumulated shares - %17.4f", cumshares);
	printf ("\n");
	printf ("\n");
	printf ("total cash received  -  %12.2f", cumcash);
	printf ("\n");
	printf ("Today's Total Return - %13.2f\n", 100 * ( currentprice * 
	    cumshares + cumcash) / (cumbasis - reinbasis) - 100);
	printf ("\n");
}


print_buy(b)
struct buy *b;
{
	float roi;

	cumbasis += b->basis;
	cumshares += b->nshares;
	printf("%2d/%2d", b->when.mm, b->when.yy);
	printf("%10.4f", b->nshares);
	printf("%10.4f", b->pershare);
	b->currentvalue = b->nshares * currentprice;
	printf("%10.2f", b->currentvalue);
	roi = (exp ((1.0 / b->heldmonths) * 
		log (b->currentvalue / b->basis)) - 1) * 100 * 12;
	printf("%10.4f", roi);
	printf("%8d", b->heldmonths);
	printf ("%12.2f", cumbasis);
	printf ("%12.4f", cumshares);
}


int
proc_buy(buf)
char *buf;
{
	register struct buy *b;

	b = &buy_trans[nbuy_trans++];
	sscanf(get_date(buf, &b->when), "%f %f %f", &b->basis, 
		&b->pershare, &b->nshares);
	b->heldmonths = curmonth - ((b->when.yy * 12) + b->when.mm);
	if(b->heldmonths == 0)
		b->heldmonths = 1;
		
}

int
proc_rein(buf)
char *buf;
{
	struct buy b;

	sscanf(get_date(buf, &b.when), "%f %f %f", &b.basis, 
		&b.pershare, &b.nshares);
	reinshares += b.nshares;
	reinbasis += b.basis;
}

int
proc_div(buf)
char *buf;
{
	struct date d;
	float dividendpercent;
	register int i;
	register struct buy *b;

	sscanf(get_date(buf, &d), "%f", &dividendpercent);
	for(i = 0, b = buy_trans; i < nbuy_trans; i++, b++)
		b->nshares += b->nshares * dividendpercent / 100.0;
}

int
proc_cash(buf)
char *buf;
{
	struct date d;
	float cashdist;

	sscanf(get_date(buf, &d), "%f", &cashdist);
	cumcash += cashdist;
}

int
proc_date(buf)
char *buf;
{
	(void) get_date(buf, &curdate);
	curmonth = curdate.yy * 12 + curdate.mm;
}

int
proc_price(buf)
char *buf;
{
	sscanf(buf, "%f", &currentprice);
}

char *
get_date(buf, date)
char *buf;
struct date *date;
{
	register int i;
	char *skip_field();

	sscanf(buf, "%d %d %d", &date->mm, &date->dd, &date->yy);
	for(i = 0; i < 3; i++)
		buf = skip_field(buf);
		
	return(buf);
}

char *
skip_field(buf)
char *buf;
{
	while(!isspace(*buf))
		buf++;
	while(isspace(*buf))
		buf++;
	return(buf);
}

print_heading()
{
	printf ("\f");
	printf ("\n\n");
	printf ("Investment Analysis Program\n");
	printf ("Todays Date - %02d/%02d/%02d\n", curdate.mm, 
		curdate.dd, curdate.yy);
	printf ("Today's Stock Price - %6.3f\n", currentprice);
	printf ("\n");
	printf ("%5s", "date");
	printf ("%10s", "#share");
	printf ("%10s", "$pershare");
	printf ("%10s", "$value");
	printf ("%10s", "%compound");
	printf ("%8s", "#months");
	printf ("%12s", "cumulative");
	printf ("%12s", "cumulative");
	printf ("\n");
	printf ("%5s", "");
	printf ("%10s", "initial");
	printf ("%10s", "");
	printf ("%10s", "current");
	printf ("%10s", "roi");
	printf ("%8s", "held");
	printf ("%12s", "basis");
	printf ("%12s", "#shares");
	printf ("\n");
	printf ("\n");
}
-- 
