#define D_UPDATE
#define D_NEWSVERBS
#define D_SCTSTR
#define D_DCHRSTR
#define D_ICHRSTR
#define D_LONSTR
#define D_FILES
#include        "empdef.h"

coll()
{
        register char   *cp;
        register        arg, i;
        char     *cname();
        int     pkgs;
        long    now, due, last, rdur,xdur;
        double  rate, owed, pay;

        arg = onearg(argp[1], "Collect on loan #");
        if( arg == -1 ) return(SYN_RETURN);
        if( getloan(arg) == -1 ) goto X102;
        if( cnum != loan.l_loner ) goto X102;
        if( loan.l_ldur != 0 ) goto X112;
X102:   
        printf("You aren't owed anything on that loan...");
        return(FAIL_RETURN);
X112:   
        time(&now);
        due = loan.l_duedate;
        last = loan.l_lastpay;
        if( now >  due ) goto X220;
        printf("There has been no default on loan %d", arg);
        return(FAIL_RETURN);
X220:   
        if( last >=  due ) goto X350;
        if( due >=  now ) goto X350;
        rdur = due - last;
        xdur = now - due;
X350:   
        if( due >=  last ) goto X434;
        rdur = 0;
        xdur = now - last;
X434:   
        rate = loan.l_irate / (loan.l_ldur * 8.64e6);
        owed = (rdur * rate + xdur * rate * 2.0+ 1.0) * loan.l_amtdue;
        printf("You are owed $%.2f on that loan.\n", owed);
        if( getsno(argp[2], "What sector do you wish to confiscate? ") == -1 ) return(SYN_RETURN);
        if( neigh(sx, sy, cnum, UP_OWN) != 0 ) goto X676;
        printf("You are not adjacent to %d,%d", sx, sy);
        return(FAIL_RETURN);
X676:   
        if( sect.sct_owned == loan.l_lonee ) goto X754;
        printf("%d,%d is not owned by %s.", sx, sy, cname(loan.l_lonee));
        return(FAIL_RETURN);
X754:   
        pay = (float)dchr[sect.sct_desig].d_value * ((float)sect.sct_effic + 100.);
        pkgs = dchr[sect.sct_desig].d_pkg;
        i = 0;
        goto X1164;
X1040:  
        if( ichr[i].i_value == 0 ) goto X1162;
        cp = (char *)((unsigned int)&sect + (unsigned int)i);
        if( *cp == '\0' ) goto X1162;
        rate = ichr[i].i_pkg[pkgs] * 10;
        pay += (float)ichr[i].i_value * *cp * rate;
X1162:  
        i++;
X1164:  
        if( ichr[i].i_name != 0 ) goto X1040;
        sigsave( );
        printf("That sector (and its contents) is valued at $%.2f\n", pay);
        if( pay <= owed ) goto X1260;
        return(FAIL_RETURN);
X1260:  
        sect.sct_owned = cnum;
        sect.sct_chkpt = sect.sct_dfend = 0;
        sect.sct_lstup = curup;
        putsect(sx, sy);
        nreport(cnum, N_SEIZE_SECT, loan.l_lonee);
        if( pay * 1.05 >= owed ) goto X1370;
        if( pay + 100. < owed ) goto X1504;
X1370:  
        loan.l_ldur = 0;
        nreport(loan.l_lonee, N_REPAY_LOAN, cnum);
        sprintf(fmtbuf,"Country %d seized %d,%d to satisfy loan %d", cnum, sx, sy, arg);
        wu(0, loan.l_lonee, fmtbuf);
        printf("That loan is now considered repaid\n");
        goto X1734;
X1504:  
        time(&loan.l_lastpay);
        owed -= pay;
        loan.l_amtdue = min(owed, 32767.);
        pay = loan.l_amtpaid + pay;
        loan.l_amtpaid = min(pay, 32767.);
        sprintf(fmtbuf,"Country %d seized %d,%d in partial payment of loan %d", cnum, sx, sy, arg);
        wu(0, loan.l_lonee, fmtbuf);
        printf("You are still owed $%.2f on loan %d\n", owed, arg);
X1734:  
        putloan(arg);
        return(NORM_RETURN);
}
