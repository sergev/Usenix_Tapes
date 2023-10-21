#
/************************************************************************/
/*									*/
/*	SS - Super ss							*/
/*									*/
/*	This program displays a number of UNIX statistics, and		*/
/*	status information. 						*/
/*	The program uses the gproc system call to access the process	*/
/*	table, and requires the CWRU version of UNIX to access the	*/
/*	the statistics gathered in kernel space.			*/
/*									*/
/*	Author:		Greg Ordy, Case Western Reserve Univ.		*/
/*	Date:		6/5/78						*/
/*	last mod:	2/3/79						*/
/*	Module:		dr.c						*/
/*	Description:	This function (main) is the main driver for SS.	*/
/*			Options:   a      :Ann Arbor terminal		*/
/*				   p	  :Display processes		*/
/*				   c	  :Display core map		*/
/*				   S	  :Display swap map		*/
/*				   s	  :processor status		*/
/*				   d	  :flip flop for long or short	*/
/*					   analysis of data in s mode	*/
/*									*/
/*				  +n	  :Sleep time between iterations*/
/*									*/
/*			Default options:  -c  +4			*/
/*									*/
/*	Entry points:	main		:entry into program		*/
/*			options		:process program options	*/
/*									*/
/************************************************************************/

#include	"df.h"
#include	"gv.h"

main (argc, argv)
int     argc;
char   *argv[];
{
    register int    i, nsleep;

/************************************************************************/
/*	Set quit to redraw screen and get options. Set interrupt to	*/
/*	erase screen and exit.						*/
/************************************************************************/

    extern int  trap (), abort ();
    signal (QUIT, trap);
    signal (INTERRUPT, abort);

/************************************************************************/
/*	If User has specified options, call options to set flags.	*/
/************************************************************************/

    nsleep = DEFSLEEP;

    for (i = 1; i != argc; i++)
    {
	switch (argv[i][0])
	{
	    case '+': 
		nsleep = atoi (&argv[i][1]);
		if (nsleep < 0 || nsleep > 10)
		    nsleep = 10;
		break;

	    case '-': 
		options (&argv[i][1]);
		break;

	    default: 
		break;

	}
    }

    initialize ();

    if ((getuid () & 0377) == NOBODY)
	nsleep = 10;

    while (FOREVER)
    {
	date ();
	for (i = 0; i++ < 60; sleep (nsleep))
	{
	    if (intflg)
	    {
		fix ();
		intflg = 0;
	    }
	    ptime ();
	    switch (modeflg)
	    {
		case TABLE: 
		    wh ();
		    break;

		case STATS: 
		    kstat ();
		    break;

		case CORE: 
		    coremap ();
		    break;

		case SWAP:
		    swap_map();
		    break;
	    }
	    bflush();
	}

    }
}

/************************************************************************/
/*	Options: This function is passed a pointer to a string of 	*/
/*	options, and chews its way through it, setting flags.		*/
/************************************************************************/

    options (optpnt)
	char   *optpnt;
    {
	while (*optpnt)
	{
	    switch (*optpnt++)
	    {
		case 'a': 
		    ttype = ANNARBOR;
		    break;
		case 'p': 
		    modeflg = TABLE;
		    break;
		case 'c': 
		    modeflg = CORE;
		    break;
		case 's':
		    modeflg = STATS;
		    break;
		case 'd':
		    if(delta == TRUE)
			delta = FALSE;
		    else
			delta = TRUE;
		    break;
		case 'S':
		    modeflg = SWAP;
		    break;
		default: 
		    break;
	    }
	}

    }
