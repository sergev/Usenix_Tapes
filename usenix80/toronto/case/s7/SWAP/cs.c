#
/************************************************************************/
/*									*/
/*	SS - Super ss							*/
/*									*/
/*	Author:		Greg Ordy, Case Western Reserve Univ.		*/
/*	Date:		6/5/78						*/
/*	Module:		cs.c						*/
/*	Description:	This function moves the cursor to the		*/
/*			specified X,Y location of a given terminal.	*/
/*			It is taken from Dave Fotland's Space Wars.	*/
/*	Entry points:	curs		:move the cursor to x,y		*/
/*									*/
/************************************************************************/

/* this subroutine moves the cuser to position x,y (x across and y
down from upper left corner. on the terminal fd of type type
where type is 1-tektronics 4023 2-beehive 3-adds 4-adm3 5-ann arbor
6-vt55
*/
curs (x, y, fd, type)
int     x, y, fd, type;
{
    char    buf[10];
    switch (type)
    {
	case 1: 
	    buf[0] = '\034';
	    buf[1] = x + 32;
	    buf[2] = y + 32;
	    rite (buf, 3);
	    break;
	case 2: 
	    buf[0] = '\033';
	    buf[1] = 'F';
	    buf[2] = '0';
	    buf[3] = x / 10 + '0';
	    buf[4] = x % 10 + '0';
	    buf[5] = '0';
	    buf[6] = y / 10 + '0';
	    buf[7] = y % 10 + '0';
	    rite (buf, 8);
	    break;
	case 6: 
	case 3: 
	    buf[0] = '\033';
	    buf[1] = 'Y';
	    buf[2] = y + 32;
	    buf[3] = x + 32;
	    rite (buf, 4);
	    break;
	case 4: 
	    buf[0] = '\033';
	    buf[1] = '=';
	    buf[2] = y + 32;
	    buf[3] = x + 32;
	    rite (buf, 4);
	    break;
	case 5: 
	    putchar (017);
	    putchar ((x > 0 ? (x / 10) * 020 + x % 10 : 0177));
	    putchar ((y < 20 ? 0100 + y : 0140 + (y - 20)));
	    break;
	default: 
	    printf ("bad term arg to curs\n");
    }
}
