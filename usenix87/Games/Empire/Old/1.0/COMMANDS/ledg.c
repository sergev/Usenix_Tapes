#define	D_LONSTR
#define	D_FILES
#include	"empdef.h"

ledg()
{
	register	i, lnum;
	char	 *cname(), *ctime();
	int	nloans;
	long	now, due, last, rdur, xdur;
	double	rate;

	time(&now);
	if( argp[1] != 0 ) {
		lnum = atoi(argp[1]);
	} else {
		lnum = -1;
		printf("\t... %s Ledger ...\n", cname(cnum));
	}
	nloans = 0;
	for( i=0; getloan(i) != -1; i++ ) {
		if( loan.l_ldur == 0 ) continue;
		if( cnum != loan.l_loner &&
		    cnum != loan.l_lonee ) continue;
		if( lnum >= 0 && i != lnum ) continue;
		nloans++;
		printf("\nLoan #%d from %s to", i, cname(loan.l_loner));
		printf(" %s\n", cname(loan.l_lonee));
		if( loan.l_amtpaid == -1 ) {
			printf("(proposed) principal=$%d interest rate=%d%%", loan.l_amtdue, loan.l_irate);
			printf(" duration(days)=%d\n", loan.l_ldur);
			if( loan.l_duedate < now ) {
				printf("This offer has expired\n");
				loan.l_ldur = 0;
				putloan(i);
				continue;
			}
			printf("Loan must be accepted by %s", ctime(&loan.l_duedate));
			continue;
		}
		last = loan.l_lastpay;
		due = loan.l_duedate;
		if( now < due ) {
			rdur = now - last;
			xdur = 0;
		}
		if( last <  due &&
		    due <  now ) {
			rdur = due - last;
			xdur = now - due;
		}
		if( due < last ) {
			rdur = 0;
			xdur = now - last;
		}
		rate = loan.l_irate / (loan.l_ldur * 8.64e6);
		printf("Amount paid to date $%d\n", loan.l_amtpaid);
		printf("Amount due (if paid now) $%.0f", (rdur * rate + xdur * rate * 2.0 + 1.0) * loan.l_amtdue);
		if( xdur == 0 ) {
			printf(" (if paid on due date) $%.0f\n", ((due - last) * rate + 1.0) * loan.l_amtdue);
			printf("Due date is %s", ctime(&loan.l_duedate));
			continue;
		}
		printf(" ** In Arrears **\n");
	}
	if( nloans <= 0 ) {
		if( lnum != -1 ) {
			printf("There is no entry in the ledger for loan #%d\n", lnum);
		} else {
			printf("The slate is clean (i.e. no entries in ledger)\n");
		}
	}
	return(NORM_RETURN);
}
