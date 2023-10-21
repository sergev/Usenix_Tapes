#
/************************************************************************/
/*									*/
/*	SS - Super ss							*/
/*									*/
/*	Author:		Greg Ordy					*/
/*	Date:		8/10/78						*/
/*	last mod:	3/23/79						*/
/*	Module:		cm.c						*/
/*	Description:    This module contains routines to display	*/
/*			the core map.					*/
/*	Entry points:	coremap		:generate the map		*/
/*			display		:fill in core size		*/
/*			clear		:clear a slot			*/
/*			name		:find the users name		*/
/*			putname		:display a name			*/
/*									*/
/************************************************************************/

#include	"df.h"
#include	"gv.h"

extern struct nl
{
	char	     lname[8];
	int	     stype;
	unsigned     value;
}
	nl[4]  ;

coremap ()
{
    register int    i, flag;
    register char   c;
    int     coreslot, status;
	extern int putname();

    for (i = 0; i != 124; i++)
	screen.csize[i] = screen.cstat[i] = ' ';

    gprocs (proc);

    for (i = 0; i < NPROC; i++)
    {
	flag = proc[i].p_flag;
	if ((flag & SLOAD) == 0)
	    continue;
	status = proc[i].p_stat;
	if (status == SZOMB)
	    continue;

	display (DATA, proc[i].p_addr >> 5, proc[i].p_size >> 5);

	switch (status)
	{
	    case SSLEEP: 
		c = 'S';
		break;
	    case SWAIT: 
		c = 'W';
		break;
	    case SRUN: 
		c = 'R';
		break;
	    case SIDL: 
		c = 'I';
		break;
	    case SSTOP: 
		c = 'T';
		break;
	}

	if (flag & SLOCK)
	    c = 'L';

	coreslot = (proc[i].p_addr >> 5) + (proc[i].p_size >> 5) / 2;
	screen.cstat[coreslot] = c;
	if (screen.ppid[coreslot] == proc[i].p_pid)
	    continue;
	name (proc[i].p_uid, coreslot, &putname);
	screen.ppid[coreslot] = proc[i].p_pid;
    }

    seek (mem, nl[1].value, 0);
    read (mem, text, sizeof text);

    for (i = 0; i != NTEXT; i++)
    {
	if (text[i].x_ccount == 0)
	    continue;

	display (TEXT, text[i].x_caddr >> 5, text[i].x_size >> 5);
	screen.cstat[(text[i].x_caddr >> 5) + (text[i].x_size >> 5) / 2] =
	    "0123456789ABCDEF"[text[i].x_count];
    }

    curs (CORE1, FD, ttype);
    rite (&screen.csize[0], 60);
    curs (STAT1, FD, ttype);
    rite (&screen.cstat[0], 60);
    curs (CORE2, FD, ttype);
    rite (&screen.csize[60], 64);
    curs (STAT2, FD, ttype);
    rite (&screen.cstat[60], 64);

    for (i = 0; i != 124; i++)
    {
	if (screen.cstat[i] == ' ' && screen.ppid[i] != -1)
	{
	    clear (i);
	    screen.ppid[i] = -1;
	}
    }
}

display (type, start, length)
int     type, start, length;
{
    if ((start + length) > 123)
	return;

    if (type == DATA)
    {
	if (length <= 1)
	{
	    screen.csize[start] = 'V';
	    return;
	}

	screen.csize[start] = '<';
	start++;
	length--;
	while (--length)
	    screen.csize[start++] = '-';

	screen.csize[start] = '>';

	return;
    }

    if (length <= 1)
    {
	screen.csize[start] = 'X';
	return;
    }

    screen.csize[start] = '[';
    start++;
    length--;
    while (--length)
	screen.csize[start++] = '=';

    screen.csize[start] = ']';

}

clear (slot)
{
    putname (slot, "        ");
}

name (who, slot, putname)
int (*putname)();
int     who, slot;
{
    register int    i, j;
    char    buf[80];

    if (who == 99)
	return;

    if (who == 0)
    {
	(*putname) (slot, "root    ");
	return;
    }

    for (i = 0; i != MAXNAME; i++)
    {
	if (uname[i].uuid == who)
	{
	    (*putname) (slot, uname[i].name);
	    return;
	}
    }

    getpw (who, buf);

    for (i = 0; i != 6; i++)
    {
	if (buf[i] == ':')
	    buf[i] = buf[i + 1] = buf[i + 2] = buf[i + 3] = ' ';
    }

    (*putname) (slot, buf);

    for (i = 0; i != MAXNAME; i++)
	if (uname[i].uuid == 0)
	    break;
    uname[i].uuid = who;
    for (j = 0; j != 6; j++)
	uname[i].name[j] = buf[j];
}

putname (slot, point)
int     slot;
char   *point;
{
    register int    j, k;
    if (slot < 60)
	k = 5;
    else
    {
	k = 15;
	slot =- 60;
    };

    for (j = 0; j != 6; j++)
    {
	curs (4 + slot, j + k, FD, ttype);
	rite (point++, 1);
    }
}
