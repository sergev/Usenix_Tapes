#define D_LONSTR
#define D_FILES
#include        "empdef.h"

extern int      xflg, wflg, mflg;

loans()
{
        register        i, k;
        char    *cp, *getstri();

        loanf = open(loanfil, O_RDWR);
X30:    
        wflg = xflg = mflg = 0;
        cp = getstri("#? ");
        if( *cp != '\0' ) goto X106;
        close(loanf);
        return;
X106:   
        i = atoi(cp);
        k = i * sizeof(loan);
        lseek(loanf, (long)k, 0);
        i = read(loanf, &loan, sizeof(loan));
        if( i >= sizeof(loan) ) goto X216;
        printf("Only %d bytes in that one...\n", i);
X216:   
        wordfix("loner", &loan.l_loner, 0);
        wordfix("lonee", &loan.l_lonee, 0);
        bytefix("irate", &loan.l_irate, 0);
        bytefix("ldur", &loan.l_ldur, 0);
        wordfix("amtpaid", &loan.l_amtpaid, 0);
        wordfix("amtdue", &loan.l_amtdue, 0);
        longfix("lastpay", &loan.l_lastpay, 0L);
        longfix("duedate", &loan.l_duedate, 0L);
        if( mflg == 0 ) goto X30;
        lseek(loanf, (long)k, 0);
        write(loanf, &loan, sizeof(loan));
        printf("Rewritten\n");
        goto X30;
}
