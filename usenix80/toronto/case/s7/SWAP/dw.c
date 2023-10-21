#
/************************************************************************/
/*									*/
/*	SS - Super ss							*/
/*									*/
/*	Author:		Greg Ordy, Case Western Reserve Univ.		*/
/*	Date:		6/5/78						*/
/*	last mod:	3/23/79						*/
/*	Module:		dw.c						*/
/*	Description:	This function draws the static characters on the*/
/*			terminal screen. As sometimes the display gets	*/
/*			messed up, hitting the quit key will signal this*/
/*			function, and redraw the screen.		*/
/*									*/
/*	Entry points:	draw		:main routine			*/
/*			dcore		:core map draw			*/
/*			dstat		:status display draw		*/
/*			rite		:buffered write			*/
/*			bflush		:buffer flush			*/
/*									*/
/************************************************************************/

#include	"df.h"
#include	"gv.h"

draw ()
{
    register int    i, j;
    char    buffer[80];

    curs (DATETITLE, FD, ttype);
    rite  ("Date: ",6);

    curs (TIMETITLE, FD, ttype);
    rite ("Time: ",6);

    curs (MESGTITLE, FD, ttype);
    if ((i = open ("/etc/sysmesg", 0)) < 0)
    {
	write (2,"Can't open sysmesg\n",19);
	return;
    }
    j = read (i, buffer, 80);
    rite (buffer, j - 1);
    close (i);

    switch (modeflg)
    {
	case TABLE: 
	    return;
	case CORE: 
	    dcore ();
	    break;
	case STATS: 
	    dstat ();
	    break;
	case SWAP:
	    dswap();
	    break;
    }
}


dcore ()
{
    curs (WHO1TITLE, FD, ttype);
    rite ("Who",3);

    curs (ADDR1TITLE, FD, ttype);
    rite ("  00********10********20********30********40********50********60   Address",74);

    curs (SIZE1TITLE, FD, ttype);
    rite ("Size",4);

    curs (STATUS1TITLE, FD, ttype);
    rite ("Status",6);

    curs (WHO2TITLE, FD, ttype);
    rite ("Who",3);

    curs (ADDR2TITLE, FD, ttype);
    rite ("  61********70********80********90********100*******110*******120* Address",74);

    curs (SIZE2TITLE, FD, ttype);
    rite ("Size",4);

    curs (STATUS2TITLE, FD, ttype);
    rite ("Status",6);

}

dstat ()
{
}

dswap()
{
	curs(STITLE,FD,ttype);
	rite("\tFree:\t\tUsed:",13);
	
	curs(W1TITLE,FD,ttype);
	rite("Who",3);
	
	curs(A1TITLE,FD,ttype);
	rite(" 00********50********100********150*******200*******250*******300   Blocks",75);
	
	curs(S1TITLE,FD,ttype);
	rite("Size",4);
	
	
	curs(W2TITLE,FD,ttype);
	rite("Who",3);
	
	curs(A2TITLE,FD,ttype);
	rite("301*******350*******400*******450********500*******550*******600    Blocks",75);
	
	curs(S2TITLE,FD,ttype);
	rite("Size",4);
	

	curs(W3TITLE,FD,ttype);
	rite("Who",3);

	curs(A3TITLE,FD,ttype);
	rite("601*******650*******700*******750********800*******850*******900    Blocks",75);

	curs(S3TITLE,FD,ttype);
	rite("Size",4);
}

rite (where, howmany)
char   *where;
{
    register char  *p1, *p2, count;
    p1 = where;
	if(qlen + howmany >= 3000)
		bflush();
    p2 = &queue[qlen];
    qlen + = howmany;
    for (count = 0; count != howmany; count++)
	*p2++ = *p1++;
}

bflush ()
{
    write (FD, queue, qlen);
    qlen = 0;
}
