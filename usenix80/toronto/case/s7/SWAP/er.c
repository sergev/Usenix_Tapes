/************************************************************************/
/*									*/
/*	SS - Super ss							*/
/*									*/
/*	Author:		Greg Ordy, Case Western Reserve Univ.		*/
/*	Date:		6/5/78						*/
/*	Module:		er.c						*/
/*	Description:	This function clears the screen of a given	*/
/*			terminal. It is taken from Dave Fotland's	*/
/*			Space Wars.					*/
/*	Entry points:	eras		:erase the screen		*/
/*									*/
/************************************************************************/

/* this program erases the screen on file descriptor fd, of type type
where type is 1-tektronics 4023 2-beehive 3-adds 4-adm3 5-ann arbor
*/
eras (fd, type)
int     fd, type;
{
    register int    i;
    switch (type)
    {
	case 1: 
	    rite ("\33\14", 2);
	    break;
	case 2: 
	    rite ("\033E", 2);
	    for (i = 0; i < 23; i++)
		rite ("                                                                                    ", 80);
	    break;
	case 3: 
	    rite ("\014", 1);
	    break;
	case 4: 
	    rite ("\032", 1);
	    break;
	case 5: 
	    rite ("\014", 1);
	    break;
	default: 
	    printf ("bad arg to erase %d", type);
    }
}
