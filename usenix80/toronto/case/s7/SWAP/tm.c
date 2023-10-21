#
/************************************************************************/
/*									*/
/*	SS - Super ss							*/
/*									*/
/*	Author:		Greg Ordy					*/
/*	Date:		6/5/78						*/
/*	last mod:	3/23/79						*/
/*	Module:		tm.c						*/
/*	Description:	This file contains two functions used in	*/
/*			displaying time information. ptime puts the	*/
/*			current time on the screen, and date inserts	*/
/*			the date.					*/
/*	Entry points:	ptime		:display current time		*/
/*			date		:display date			*/
/*									*/
/************************************************************************/

#include	"df.h"
#include	"gv.h"

ptime ()
{
    int     tvec[2];
    register char  *pnt;

    time (tvec);
    pnt = ctime (tvec);
    curs (TIMEDATA, FD, ttype);
    pnt =+ 11;
    rite (pnt, 8);
}


date ()
{
    int     tvec[2];
    register char  *pnt;

    time (tvec);
    pnt = ctime (tvec);
    curs (DATEDATA, FD, ttype);
    rite (pnt, 10);
    rite (",", 1);
    pnt =+ 19;
    rite (pnt, 5);
}
