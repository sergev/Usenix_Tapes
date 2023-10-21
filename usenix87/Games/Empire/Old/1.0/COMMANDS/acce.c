#define D_NEWSVERBS
#define D_NATSTR
#define D_LONSTR
#define D_FILES
#include	"empdef.h"

acce()
{
	register char	*cp;
	register	loanum;
	char	 *ctime(), *cname(), *getstar();

	loanum = onearg(argp[1], "loan number? ");
	if( loanum == -1 ) return(SYN_RETURN);
	getloan(loanum);
	if( loan.l_ldur == 0 ||
	    cnum != loan.l_lonee ) {
		printf("Loan %d is not being offered to you!", loanum);
		return(SYN_RETURN);
	}
	if( loan.l_amtpaid != -1 ) {
		printf("That loan has already been accepted!");
		return(FAIL_RETURN);
	}
	printf("Loan %d offered by %s on %s", loanum, cname(loan.l_loner), ctime(&loan.l_lastpay));
	printf("principal $%d  interest rate %d%%  duration (days) %d\n", loan.l_amtdue, loan.l_irate, loan.l_ldur);
	printf("This offer will be retracted if not accepted by %s", ctime(&loan.l_duedate));
X302:	
	cp = getstar(argp[2], "Accept, decline or postpone? ");
	argp[2] = 0;
	switch( *cp ) {
	case 'a':
		getnat(loan.l_loner);
		if( loan.l_amtdue >  nat.nat_money ) {
			loan.l_amtdue = nat.nat_money;
			printf("%s no longer has the funds...\n", nat.nat_cnam);
			printf("You may borrow $%d at the", loan.l_amtdue);
			printf(" same terms\n");
			goto X302;
		}
		nat.nat_money -= loan.l_amtdue;
		sigsave();
		putnat(loan.l_loner);
		getnat(cnum);
		nat.nat_money += loan.l_amtdue;
		putnat(cnum);
		loan.l_amtpaid = 0;
		time(&loan.l_lastpay);
		loan.l_duedate =  loan.l_ldur * 86400L + loan.l_lastpay;
		putloan(loanum);
		printf("You are now $%d richer (sort of)\n", loan.l_amtdue);
		sprintf(fmtbuf,"Loan %d accepted by country #%d", loanum, cnum);
		wu(0, loan.l_loner, fmtbuf);
		nreport(loan.l_loner, N_MAKE_LOAN, cnum);
		break;
	case 'd':
		loan.l_ldur = 0;
		putloan(loanum);
		sprintf(fmtbuf,"Loan %d refused by country #%d", loanum, cnum);
		wu(0, loan.l_loner, fmtbuf);
		printf("Loan %d refused\n", loanum);
		break;
	case 'p':
		printf("Okay...\n");
		sprintf(fmtbuf,"Loan %d considered by country #%d", loanum, cnum);
		wu(0, loan.l_loner, fmtbuf);
		break;
	default:
		printf("Stop talking gibberish!!! ");
		goto X302;
	}
	return(NORM_RETURN);
}
