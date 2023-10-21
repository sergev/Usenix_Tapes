:
#! /bin/awk -f
#
# program:	roi
# 
# authors:	gary s. anderson
#		steve swift mod 12/85
#		steve swift mod 9/85
#
# date:		14 february 1984
#
# purpose:	to analyze stock and mutual fund investments
#		This version shortens the output for 80 column
#		terminals and keeps track of distributions
#		of cash and reinvested distributions.
#
# description:	tbc
#
# input file format:
#		the first record of the input must indicate today's date
#		in the form
#
#			date	mm	dd	yy
#
#		where date is literally 'date' and mm, dd, and yy are as you 
#		would expect.
#
#		the second record is of the form
#
#			price	xxx.xx
#
#		where price is literally 'price' and xxx.xx is today's market
#		value of the stock.
#
#		a purchase transaction has the form
#
#			buy  mm  dd  yy  bbb.bb  ppp.pp  nnn.nn
#
#		where buy is literally 'buy', mm, dd, and yy are the date of
#		the purchase transaction, bbb.bb is the amount of dollars
#		invested, ppp.pp is the per share price, and nnn.nn is the
#		number of shares purchased.
#
#		a stock dividend transaction has the form
#
#			div  mm  dd  yy  ppp.pp
#
#		where div is literally 'div' and ppp.pp is the percentage of
#		dividend awarded.
#
#		A reinvested cash distribution has the form
#
#			rein  mm  dd  yy  bbb.bb  ppp.pp  nnn.nn
#
#		where rein is literally "rein' and the rest are treated
#		the same as a buy transaction.  Any reinvested gain
#		must also be recorded in a buy record for basis to be
#		correctly calculated.
#
#		A cash distribution (that is not reinvested) has the form
#
#			cash  mm  dd  yy  bbb.bb
#
#		where cash is literally 'cash' and bbb.bb is the amount of
#		cash received.
#
#
# initialize the transaction array index
BEGIN	\
   	{
	printf ("Stock Investment Program ");
	printf ("\n");
	purchase = 0;
	#set the processing flag to enable processing of buy transactions
	processing = 1;
	#clear the selling data flag to disable selling output data
	selling = 0;
   	}

#ignore commented lines
/#/ \
	{
	print "comment--- ", $0;
	next;
	}

# process a purchase transaction
/buy/	\
	{
	if (processing == 1)
	    {
	    purchase = purchase +1;
	    transactiontype [purchase] = "purchase";
	    date [purchase] = $4*12+$2;
	    heldmonths [purchase] = currentdate - date[purchase];
	    if (heldmonths [purchase] == 0) heldmonths [purchase] = 1;
	    basis [purchase] = $5;
	    pricepershare [purchase] = $6;
	    initshares [purchase] = $7;
	    currentshares [purchase] = $7;
	    month [purchase] = $2;
	    year [purchase] = $4;
	    }
	}

# process a reinvested distribution transaction
/rein/	\
	{
	if (processing == 1)
	    {
	    dist = dist +1;
	    disttranstype [dist] = "dist";
	    distbasis [dist] = $5;
	    distinitshares [dist] = $7;
	    distcurshares [dist] = $7;
	    }
	}

/cash/	\
	{
	    coin = coin +1;
	    cointranstype [coin] = "coin";
	    coinbasis [coin] = $5;
	}
# process a dividend transaction
/div/	\
	{
	dividendpercent = $5;
	for (i=1; i<=purchase; i=i+1)
	    {
	    currentshares [i] = currentshares[i]  \
		+ currentshares[i]*dividendpercent/100;
	    }
	}

# process the sold start or end record
/sold/ 	\
	{
	print $0
	print $2
	if ($2 == "start")
	    {
	    # disable further processing of buy transactions
	    processing = 0;
	    }
	if ($2 == "end")
	    {
	    # enable processing of buy transactions
	    processing = 1;
	    }
	}

# process the selling data output traction
/selling/ \
	{
	# enable the data output of selling data
	selling = 1;
	sharessold = $2;
	}

# process the date record
/date/	\
	{
	currentdate = $4*12+$2;
	todaymonth = $2;
	todayday = $3;
	todayyear = $4;;
	}

# process the price record
/price/	\
	{
	currentprice = $2;
	}

# all the data is now collected, output the reports
END	\
	{
	cumulativeshares = 0;
	cumulativebasis = 0;
	for (i=1; i<=purchase; i=i+1)
	    {
	    if ((i % 40) == 1)
		{
		printf ("");
		printf ("\n\n");
		printf ("Investment Analysis Program\n");
		printf ("Todays Date - %2d/%2d/%2d\n", todaymonth, todayday, \
		    todayyear);
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
	    currentvalue [i] = currentshares[i] * currentprice;
	    # the following uses the fact that x^y = exp(y*log(x))
	    roi [i]  = (exp ((1.0 / heldmonths [i]) * \
		log (currentvalue [i] / basis [i])) - 1) * 100 * 12;
	    cumulativeshares = cumulativeshares + currentshares [i];
	    cumulativebasis = cumulativebasis + basis [i];
	    printf ("%2d/%2d ", month [i], year [i]);
	    printf ("%10.4f", initshares [i]);
	    printf ("%10.4f", pricepershare [i]);
	    printf ("%10.2f", currentvalue [i]);
	    printf ("%10.4f", roi [i]);
	    printf ("%8d", heldmonths [i]);
	    printf ("%12.2f", cumulativebasis);
	    printf ("%12.4f", cumulativeshares);
	    if (selling == 1)
		{
		printf ("(-%5d = %12.4f)",  sharessold, cumulativeshares-sharessold);
		}
	    printf ("\n");
	    }
        printf ("\n");
        printf ("\n");

#	all the cash dividend data is collected

	cumcoinbasis = 0;
	for (i=1; i<=coin; i=i+1)
	    {
	    cumcoinbasis = cumcoinbasis + coinbasis [i];
	    }

# all the dist  data is now collected, output the reports
	cumdistshares = 0;
	cumdistbasis = 0;
	for (i=1; i<=dist; i=i+1)
	    {
	    cumdistshares = cumdistshares + distcurshares [i];
	    cumdistbasis = cumdistbasis + distbasis [i];
	    }
            printf ("distribution dollars -  %12.2f", cumdistbasis);
	    printf ("\n");
            printf ("distribution shares  -  %14.4f", cumdistshares);
	    printf ("\n");
	    printf ("\n");
            printf ("total investment dollars -  %8.2f", cumulativebasis - \
                                  cumdistbasis);
	    printf ("\n");
	    printf ("total investment shares  -  %10.4f", cumulativeshares - \
                                  cumdistshares); 
	    printf ("\n");
	    printf ("\n");
	    printf ("total value - %22.2f", currentprice * cumulativeshares);
	    printf ("\n");
            printf ("accumulated shares - %17.4f", cumulativeshares);
	    printf ("\n");
	    printf ("\n");
            printf ("total cash received  -  %12.2f", cumcoinbasis);
	    printf ("\n");
	    printf ("Today's Total Return - %13.2f\n", 100*( currentprice * \
	            cumulativeshares + cumcoinbasis) / (cumulativebasis - cumdistbasis));
	    printf ("\n");
        }

_________________________________________________________________________

-- 

Steven Swift       (206) 356-5278
John Fluke Mfg. Co.
P.O. Box C9090  Everett WA  98206  
{uw-beaver,decvax!microsof,ucbvax!lbl-csam,allegra,ssc-vax}!fluke!swifty
