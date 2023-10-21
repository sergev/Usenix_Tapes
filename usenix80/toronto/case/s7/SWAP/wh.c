#
/************************************************************************/
/*									*/
/*	SS - Super ss							*/
/*									*/
/*	Author:		Greg Ordy					*/
/*	Date:		7/11/78						*/
/*	last mod:	3/23/79						*/
/*	Module:		wh.c						*/
/*	Description:	This module contains the routines that		*/
/*			dynamically display the process table.		*/
/*	Entry points:	wh		:driver				*/
/*			gpname		:get process name		*/
/*			checkbuf	:purify a process name		*/
/*			itoa		:integer to ascii		*/
/*									*/
/************************************************************************/

#include	"df.h"
#include	"gv.h"

wh ()
{
    register int    i, j, k;
    char    buf[100];
    int     n;

    for (i = 0; i != SLOTS; i++)
	cproc[i].used = FALSE;

    gprocs (proc);

    for (i = 0; i != NPROC; i++)
    {
	if (proc[i].p_stat == 0)
	    continue;
	if (proc[i].p_uid == 99)
	    continue;

	for (j = 0; j != SLOTS; j++)
	{
	    if (proc[i].p_pid == cproc[j].pid)
	    {
		if (proc[i].p_pid == SZOMB && cproc[j].flags == 1)
		{
		    curs (19, j + 3, FD, ttype);
		    rite ("ZOMBIE                                              ", 30);
		    cproc[j].flags++;
		    break;
		}
		if (cproc[j].unknown == TRUE)
		{
		    n = gpname (i, buf) - 2;
		    n = checkbuf (n, buf);
		    if (buf[0] == '-')
			break;
		    curs (19, j + 3, FD, ttype);
		    rite (buf, n);
		    cproc[j].unknown = FALSE;
		}
		break;
	    }
	}

	if (j != SLOTS)
	{
	    cproc[j].used = TRUE;
	    continue;
	}

	if (proc[i].p_stat == SZOMB)
	    continue;
	getpw (proc[i].p_uid, buf);
	for (j = 0; j != SLOTS - 1; j++)
	    if (cproc[j].flags == 0)
		break;

	for (k = 0; k != 11; k++)
	{
	    if (buf[k] == ':')
	    {
		buf[k] = ' ';
		buf[k + 1] = ':';
	    }
	}

	curs (1, j + 3, FD, ttype);
	rite (buf, 11);

	rite (itoa (proc[i].p_pid), 5);
	rite ("  ", 2);

	if (proc[i].p_pid == 0)
	    rite ("System Scheduler", 16);
	else
	{
	    n = gpname (i, buf) - 2;
	    n = checkbuf (n, buf);
	    if (buf[0] == ' ' && buf[1] == ' ' && buf[2] == ' ')
		rite ("???                                                ", 30);
	    else
		rite (buf, n);
	    if (buf[0] == '-' && buf[1] == ' ')
		cproc[j].unknown = TRUE;
	}

	cproc[j].used = TRUE;
	cproc[j].pid = proc[i].p_pid;
	cproc[j].flags = 1;

    }

    for (i = 0; i != SLOTS; i++)
    {
	if (cproc[i].used == FALSE && cproc[i].flags != 0)
	{
	    curs (1, i + 3, FD, ttype);
	    rite ("                                                                                    ", 68);
	    cproc[i].flags = 0;
	}
    }

}

gpname (which, bufpnt)
int     which;
char   *bufpnt;
{
    int     laddr, baddr, mf, stbuf[257];
    register int   *ip, *cp, n;

    baddr = 0;
    laddr = 0;

    if (proc[which].p_flag & SLOAD)
    {
	laddr = proc[which].p_addr;
	mf = mem;
    }
    else
    {
	baddr = proc[which].p_addr;
	mf = swap;
    }
    laddr =+ proc[which].p_size - 8;
    baddr =+ laddr >> 3;
    laddr = (laddr & 07) << 6;
    seek (mf, baddr, 3);
    seek (mf, laddr, 1);
    read (mf, stbuf, 512);
    for (ip = &stbuf[256]; ip > &stbuf[0];)
	if (*--ip == -1)
	{
	    ip++;
	    cp = bufpnt;
	    n = 20;
	    do
		*cp++ = *ip++;
	    while (--n);

	    if (bufpnt[0] < ' ' || bufpnt[0] > 'z')
	    {
		bufpnt[0] = '?';
		bufpnt[1] = ' ';
		return (3);
	    }
	    if (bufpnt[0] == '-' && bufpnt[1] < ' ')
	    {
		bufpnt[1] = bufpnt[2] = ' ';
		return (3);
	    }

	    for (mf = 0; mf != 40; mf++)
	    {
		if (bufpnt[mf] == 0)
		{
		    bufpnt[mf] = ' ';
		    continue;
		}
		if (bufpnt[mf] < ' ' || bufpnt[mf] > 'z')
		{
		    bufpnt[mf] = ' ';
		    if (mf < 5)
		    {
			bufpnt[0] = '?';
			bufpnt[1] = '?';
			bufpnt[2] = ' ';
			return (4);
		    }
		    return (mf - 1);
		}

	    }
	    return (mf - 1);
	}
}

checkbuf (i, buffer)
register char  *buffer;
{
    register int    n;

    n = i;

    while (n-- != 0)
    {
	if (*buffer < ' ' || *buffer > 'z')
	{
	    *buffer++ = '-';
	    *buffer++ = ' ';
	    return (4);
	}
	buffer++;
    }
    return (i);
}

itoa (value)
{
    register char  *p;
    register int    j, mflag;
    static char buf[12];

    mflag = 1;
    for (j = 0; j < 11; j++)
	buf[j] = ' ';
    p = buf + 6;
    if (value == 0)
    {
	*p = '0';
	p--;
    }
    if (value < 0)
    {
	mflag = -1;
	value * = -1;
    }
    while (value != 0)
    {
	*p-- = value % 10 + 060;
	value / = 10;
    }
    if (mflag == -1)
	*p-- = '-';
    return (++p);
}
