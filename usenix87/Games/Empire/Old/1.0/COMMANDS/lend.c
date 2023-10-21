#define	D_LONSTR
#define	D_NATSTR
#define	D_FILES
#include	"empdef.h"

lend()
{
	register	i, j;
	int	arg;

	if( (arg = natarg(argp[1], "lend to ?")) == -1 ) return(SYN_RETURN);
	getnat(cnum);
	i = (nat.nat_money > 32767.) ? 32767 : nat.nat_money;
	sprintf(fmtbuf,"Size of loan for country #%d? (max $%d) ", arg, i);
	j = onearg(argp[2], fmtbuf);
	if( j != 0 ) goto X166;
	return(FAIL_RETURN);
X166:	
	if( j <= i ) goto X202;
	printf("You haven't got the cash...");
	return(FAIL_RETURN);
X202:	
	i = 0;
	goto X216;
X206:	
	if( loan.l_ldur == 0 ) goto X232;
	i++;
X216:	
	if( getloan(i) != -1 ) goto X206;
X232:	
	loan.l_loner = loan.l_lonee = 0;
	loan.l_ldur = 1;
	putloan(i);
	loan.l_amtdue = j;
	j = onearg(argp[3], "Duration? (days, max 127) ");
	loan.l_ldur = max127(j);
	j = onearg(argp[4], "Interest rate? (max 127%) ");
	loan.l_irate = max127(j);
	loan.l_loner = cnum;
	loan.l_lonee = arg;
	loan.l_amtpaid = -1;
	time(&loan.l_lastpay);
	loan.l_duedate = loan.l_ldur * 86400L + loan.l_lastpay;
	putloan(i);
	printf("You have offered loan %d\n", i);
	sprintf(fmtbuf,"Country #%d has offered you a loan (#%d)", cnum, i);
	wu(0, arg, fmtbuf);
	return(NORM_RETURN);
}
