#define	D_LONSTR
#define	D_NATSTR
#define	D_NEWSVERBS
#define	D_FILES
#include	"empdef.h"

repa()
{
	register	arg;
	long	now, due, last, rdur, xdur;
	double	rate, owed, pay;

	if( (arg = onearg(argp[1], "repay loan #")) == -1) return(SYN_RETURN);
	if( getloan(arg) == -1 ) goto X102;
	if( cnum != loan.l_lonee ) goto X102;
	if( loan.l_ldur != 0 ) goto X112;
X102:	
	printf("You don't owe anything on that loan...");
	return(FAIL_RETURN);
X112:	
	time(&now);
	due = loan.l_duedate;
	last = loan.l_lastpay;
	if( now >=  due ) goto X240;
	rdur = now - last;
	xdur = 0;
X240:	
	if( last >=  due ) goto X370;
	if( due >=  now ) goto X370;
	rdur = due - last;
	xdur = now - due;
X370:	
	if( due >=  last ) goto X454;
	rdur = 0;
	xdur = now - last;
X454:	
	rate = loan.l_irate / (loan.l_ldur * 8.64e6);
	owed = (rdur * rate + xdur * rate * 2.0 + 1.0) * loan.l_amtdue;
	sprintf(fmtbuf,"You presently owe $%.0f    payment : $", owed);
	pay = onearg(argp[2], fmtbuf);
	if( pay >  owed + 1.0 || pay < 0 ) {
		printf("You don't owe that much...");
		return(FAIL_RETURN);
	}
	if( pay == 0 ) return(SYN_RETURN);
	getnat(cnum);
	if( pay > nat.nat_money ) {
		printf("You only have $%.0f...", nat.nat_money);
		return(FAIL_RETURN);
	}
	nat.nat_money -= pay;
	sigsave();
	putnat(cnum);
	getnat(loan.l_loner);
	nat.nat_money += pay;
	putnat(loan.l_loner);
	time(&loan.l_lastpay);
	if( pay + 1.0 >= owed ) {
		sprintf(fmtbuf,"Country #%d paid off loan %d with $%.0f", cnum, arg, pay);
		wu(0, loan.l_loner, fmtbuf);
		nreport(cnum, N_REPAY_LOAN, loan.l_loner);
		loan.l_ldur = 0;
		printf("Congratulations, you've paid off the loan!\n");
	} else {
		sprintf(fmtbuf,"Country #%d paid $%.0f on loan %d", cnum, pay, arg);
		wu(0, loan.l_loner, fmtbuf);
		loan.l_amtdue = min(owed - pay, 32767.);
		pay = loan.l_amtpaid + pay;
		loan.l_amtpaid = min(pay, 32767.);
	}
	putloan(arg);
	return(NORM_RETURN);
}
