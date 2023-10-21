#
/************************************************************************/
/*									*/
/*	SS - Super ss							*/
/*									*/
/*	Author:		Greg Ordy, Case Western Reserve Univ.		*/
/*	Date:		6/6/78						*/
/*	last mod:	3/23/79						*/
/*	Module:		in.c						*/
/*	Description:	This function is called to get the users	*/
/*			terminal type, which is put in variable ttype.	*/
/*	Entry points:	initialize	:initialize			*/
/*			trap		:called as IRS to QUIT		*/
/*			fix		:erase screen and get options	*/
/*			abort		:erase screen and terminate	*/
/*									*/
/************************************************************************/

#include	"df.h"
#include	"gv.h"

char	*corefile	"/dev/mem",
	*swapdev	"/dev/rk0";

struct nl
{
	char	     lname[8];
	int	     stype;
	unsigned     value;
}
	nl[5]  {
			"_swapmap",0,0,
			"_text",0,0,
			"_nswap",0,0,
			"_swplo",0,0,
			0,0,0
		};

initialize ()
{
    register int    i, j;
    int     fd;
    register char   c;
    struct
    {
	char    status,		/* terminal status */
	        which,		/* which terminal */
	        terminaltype,	/* terminal type */
	        newline;	/* newline at end of entry */
    }
            ttysform;

/************************************************************************/
/*	If terminal type is not ANNARBOR, get type through /etc/ttys.	*/
/************************************************************************/

    if (ttype != ANNARBOR)
    {
	if ((c = ttyn (FD)) == 'x')
	{
	    printf ("Input must be from a terminal.\n");
	    exit ();
	}
	if ((fd = open ("/etc/ttys", 0)) < 0)
	{
	    printf ("Cannot open /etc/ttys.\n");
	    exit ();
	}

	while (read (fd, &ttysform, sizeof ttysform) == sizeof ttysform)
	    if (ttysform.which == c)
		break;

	ttype = ttysform.terminaltype - '0';

	if (ttype != 2 && ttype != 4)
	{
	    printf ("Cannot run on this type of terminal.\n");
	    exit ();
	}

	if (ttype == 4)
	    ttype = 3;

	close (fd);
    }

    eras (FD, ttype);
    draw ();

/************************************************************************/
/*	Initialize nl, and open files.					*/
/************************************************************************/

    nlist ("/unix", nl);
    if ((mem = open (corefile, 0)) < 0 || (swap = open (swapdev, 0)) < 0)
    {
	printf ("\nEither corefile or swap device cannot be opened.\n");
	exit ();
    }


/************************************************************************/
/*	The coremap routine needs all of its pid slots set to -1, the	*/
/*	value they have when a slot is blank.				*/
/************************************************************************/

    for (i = 0; i != 180; i++)
	screen.ppid[i] = -1;

/***********************************************************************/
/*	Initialize some structures for wh.				*/
/************************************************************************/

    for (i = 0; i != SLOTS; i++)
    {
	cproc[i].pid = -1;
	cproc[i].flags = 0;
    }

	/* swap map initializations */

	seek(mem,nl[3].value,0);
	read(mem,&swplo,sizeof(swplo));

	seek(mem,nl[2].value,0);
	read(mem,&nswap,sizeof(nswap));
}

trap ()
{
    signal (QUIT, trap);
    intflg = TRUE;
}

fix ()
{
    register int    i;
    register char   c;
    char    buffer[30];

    i = 0;
    putchar (07);
    curs (1, 2, FD, ttype);
    rite ("OPTIONS: ", 9);
    bflush();
    while ((c = getchar ()) != '\n')
	buffer[i++] = c;
    buffer[i] = 0;
    if (i > 0)
	options (buffer);
    eras (FD, ttype);
    draw ();
    for (i = 0; i != 180; i++)
	screen.ppid[i] = -1;
    date ();
    for (i = 0; i != SLOTS; i++)
    {
	cproc[i].pid = -1;
	cproc[i].flags = 0;
    }
}

abort ()
{
    eras (FD, ttype);
    bflush();
    exit ();
}
