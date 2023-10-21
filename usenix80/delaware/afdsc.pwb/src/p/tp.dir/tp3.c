#
#include "tp.h"

/*

Name:
	gettape

Function:
	Process tape.

Algorithm:
	For each command line file, scan core directory for name.
	Report if not found.  Otherwise process file with argument routine.

Parameters:
	Routine to process file.

Returns:
	None.

Files and Programs:
	None.


*/
gettape (how)
int     (*how) ();
{
    register char  *ptr0,
                   *ptr1;
    register struct dent   *d;
    int     count;

    do
    {
	d = &dir[0];
	count = 0;
	do
	{
	    if (d -> d_namep == 0)
		continue;
	    decode (name, d);
	    if (rnarg > 0)
	    {
		ptr0 = &name;
		ptr1 = *parg;
		while (*ptr1)
		    if (*ptr0++ != *ptr1++)
			goto cont;
		if (*ptr0 && *ptr0 != '/')
		    goto cont;
	    }
	    (*how) (d);					/* delete, extract, or taboc */
	    ++count;
    cont: 
	    continue;
	}
	while (++d <= lastd);
	if (count == 0 && rnarg > 0)
	    printf ("%s  not found\n", *parg);
	++parg;
    }
    while (--narg > 0);
}

/*

Name:
	delete

Function:
	Delete command.

Algorithm:
	Clear entry of deleted item.

Parameters:
	Pointer to directory item.

Returns:
	None

Files and Programs:
	None.


*/
delete (dd)
{
    if (verify ('d') >= 0)
	clrent (dd);
}

/*

Name:
	update

Function:
	Update command.

Algorithm:
	Construct bitmap of used tape blocks.
	Scan core directory, skipping fake entries(mode but no name,
	or zero length).  Get target size in rounded up blocks.  Look for
	free section of tape via bitmap that could hold file.  Report tape
	overflow.  Mark bitmap that tape section is used, write directory,
	and update actual file.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/

update ()
{
    register struct dent   *d;
    register    b,
                last;
    int     first,
            size;


    bitmap ();
    d = &dir[0];
    do
    {
	if (d -> d_namep == 0 || d -> d_mode >= 0)
	    continue;
	if (d -> d_size1 == 0 && d -> d_size0 == 0)
	    continue;
							/* find a place on the tape for this file */
	size = BSIZE;
	if (d -> d_size1 & 511)
	    ++size;
	first = ndentd8;
toosmall: 
	++first;
	if ((last = first + size) >= tapsiz)
	    maperr ();
	for (b = first; b < last; ++b)
	    if (map[(b >> 3) & ~0160000] & (1 << (b & 7)))
	    {
		first = b;
		goto toosmall;
	    };
	d -> d_tapea = first;
	setmap (d);
    }
    while (++d <= lastd);
    wrdir ();
    update1 ();
}
/*

Name:
	update1

Function:
	Actual update command.

Algorithm:
	Scan core directory for entry to be updated with lowest tape address.
	Mark entry as having been updated, get name from corre buffer anad open
	it.  Report error on open or during further I/O.  Write file to tape
	at found address, including last partial block.  Close file.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/

update1 ()
{
    register struct dent   *d;
    register    index,
                id;
	int owner;

    for (;;)
    {
	d = &dir[0];
	index = 32767;
	id = 0;
	do
	{						/* find new dent with lowest tape address */
	    if (d -> d_namep == 0 || d -> d_mode >= 0)
		continue;
	    if (d -> d_tapea < index)
	    {
		index = d -> d_tapea;
		id = d;
	    }
	}
	while (++d <= lastd);
	if ((d = id) == 0)
	    return;
	d -> d_mode =& ~0100000;			/* change from new to old */
	if (d -> d_size1 == 0 && d -> d_size0 == 0)
	    continue;
	decode (name, d);
	wseek (index);
	if ((id = open (name, 0)) < 0)
	{
	    printf ("Can't open %s\n", name);
	    continue;
	}
	for (index = BSIZE; index != 0; --index)
	{
	    if (read (id, tapeb, 512) != 512)
		phserr ();
	    twrite ();
	}
	if (index = d -> d_size1 & 511)
	{
	    if (read (id, tapeb, index) != index)
		phserr ();
	    twrite ();
	}
	if (read (id, tapeb, 1) != 0)
	    phserr ();
	close (id);
    }
}

/*

Name:
	phserr

Function:
	Report phase error.

Algorithm:
	Print error message.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
phserr ()
{
    printf ("%s -- Phase error \n", name);
}


/*

Name:
	bitmap

Function:
	Initialize bitmap.

Algorithm:
	ZZ ero out map space.  Scan core directory, marking all used blocks.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
bitmap ()						/* place old files in the map */
{
    register   *m,
                count;
    register struct dent   *d;

    m = &map;
    count = sizeof map / 2;
    do
	*m++ = 0;
    while (--count);
    count = ndirent;
    d = &dir[-1];
    do
    {
	d++;
	if (d -> d_namep == 0 || d -> d_mode < 0)
	    continue;
	if (d -> d_size1 || d -> d_size0)
	    setmap (d);
    }
    while (--count);
}

/*

Name:
	setmap

Function:
	Mark bitmap.

Algorithm:
	Get size of file in rounded up blocks.  Check for valid tape address.
	Mark map to reserve blocks.

Parameters:
	Pointer to directory entry.

Returns:
	None.

Files and Programs:
	None.


*/
setmap (dd)
{
    register char  *c,
                   *block,
                   *d;
    char    bit;

    d = dd;						/* used for dent pointer then as map index */
    c = BSIZE;
    if (d -> d_size1 & 511)
	c++;
    block = d -> d_tapea;
    if ((c =+ block) >= tapsiz)
	maperr ();
    do
    {
	bit = 1 << (block & 7);
	d = (block >> 3) & ~0160000;
	if (bit & map[d])
	    maperr ();
	map[d] =| bit;
    }
    while (++block < c);
}

/*

Name:
	maperr

Function:
	Report map error.

Algorithm:
	Print message and exit.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
maperr (nnnnn)
{
    printf ("Tape overflow\n");
    done (EOTHER);
}

/*

Name:
usage

Function:
	Print stats.

Algorithm:
	Construct final bitmap of tape block usage.  Scan core directory
	for count of entries.  Find last used block.  If DECtape, count free
	blocks left.  Display the count info.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/

usage ()
{
    register    reg,
                count,
                d;
    int     nused,
            nentr,
            nfree;
    static  lused;

    bitmap ();
    d = &dir[0];
    count = ndirent;
    reg = 0;
    do
    {
	if (d -> d_namep != 0)
	    reg++;
	d =+ sizeof dir[0];
    }
    while (--count);
    nentr = reg;
    d = 0;						/* used to count nused */
    reg = ndentd8;
    ++reg;						/* address of first non-directory tape block */
    count = tapsiz - reg;
    do
    {
	if (reg >= tapsiz)
	{
	    printf ("Tape overflow\n");
	    done (EOTHER);
	}
	if (map[(reg >> 3) & ~0160000] & (1 << (reg & 7)))
	{
	    d++;
	    lused = reg;
	}
	else
	{
	    if (flags & flm)
		break;
	    nfree++;
	}
	reg++;
    }
    while (--count);
    nused = d;
    printf ("%5d entries\n%5d used\n", nentr, nused);
    if ((flags & flm) == 0)
	printf ("%5d free\n", nfree);
    printf ("%5d last\n", lused);
}
/*

Name:
	taboc

Function:
	Table of contents command.

Algorithm:
	Print entry file name.  If verbose, also print permissions, tape
	address, owner, size, and date info.

Parameters:
	Pointer to directory entry.

Returns:
	None.

Files and Programs:
	None.


*/
taboc (dd)
{
    register    mode;
    register   *m;
    register char  *s;
    int     count;

    if (flags & flv)
    {
	mode = dd -> d_mode;
	s = &catlb[9];
	*s = 0;
	for (count = 3; count; --count)
	{
	    if (mode & 1)
		*--s = 'x';
	    else
		*--s = '-';
	    if (mode & 2)
		*--s = 'w';
	    else
		*--s = '-';
	    if (mode & 4)
		*--s = 'r';
	    else
		*--s = '-';
	    mode =>> 3;
	}
	if (mode & 4)
	    s[2] = 's';
	if (mode & 2)
	    s[5] = 's';
	m = locv (dd -> d_size0 & 255, dd -> d_size1);
	printf ("%s%4d%4d%6d%9s ", s, dd -> d_uid, dd -> d_gid,
		dd -> d_tapea, m);
							/* funny business below is to make all digits pad with leading
							   zeros instead of blanks */
	s = date (dd -> d_time);
	printf ("%s ", s);
    }
    printf ("%s\n", name);
}
/*

Name:
	extract

Function:
	Extract command.

Algorithm:
	If fake entry (zero size) return.  Set output to standard output or
	to named file.  Create any needed directories to get ot file.  Copy
	from tape to output file by block, including last partial block.  Leave
	output file as owned by user, unless requested to preserve original
	owner information.

Parameters:
	Pointer to directory entry.

Returns:
	None.

Files and Programs:
	None.


*/


extract (dd)
{
    register   *d,
                count,
                id;
	int owner;

    d = dd;
    if (d -> d_size0 == 0 && d -> d_size1 == 0)
	return;
    if (verify ('x') < 0)
	return;
    rseek (d -> d_tapea);
	if(flags&flo) {
		id = 1;
	}
	else {
		unlink (name);
		owner = (d -> d_gid << 8) | (d -> d_uid & 511);
		if ((id = creat (name, d -> d_mode)) < 0) {
			mkdir (name,owner);					/* make any necessary directories */
			if ((id = creat (name, d -> d_mode)) < 0) {
				printf ("%s -- create error\n", name);
				return;
			}
		}
	}
    count = BSIZE;
    while (count--)
    {
	tread ();
	if (write (id, tapeb, 512) != 512)
	    goto ng;
    }
    if (count = d -> d_size1 & 511)
    {
	tread ();
	if (write (id, tapeb, count) != count)
	{
    ng: 
	    printf ("%s -- write error\n", name);
		if((flags&flo) == 0)
			close (id);
	    return;
	}
    }
	if(flags&flo)
		return;
    close (id);
	if(flags&flp)
		chown (name, owner);
}

/* name:
	date

function:
	to get a date format as desired for tp

algorithm:
	Get the unix format date and jumble it around to be corect format
	Unix format is:
	day mmm dd hh:mm:ss yyyy
	our format is:
	dd mmm yyyy hh:mm

parameters:
	pointer to the doubleword time

Returns:
	Pointer to the date format corresponding to that time

 */
char   *date (tvec)
int     tvec[2];
{
    register int   *i;
    register char  *p;
    register char  *t;
    static char dbuf[18];

    t = ctime (tvec);
    p = dbuf;

    *p++ = t[8];					/* tens of day */
    *p++ = t[9];					/* units of day */
    *p++ = ' ';
    *p++ = t[4];					/* month */
    *p++ = t[5];					/* month */
    *p++ = t[6];					/* month */
    *p++ = ' ';
    *p++ = t[22];					/* year */
    *p++ = t[23];					/* year */
    *p++ = ' ';
    *p++ = ' ';
    *p++ = t[11];
    *p++ = t[12];
    *p++ = ':';
    *p++ = t[14];
    *p++ = t[15];
    *p++ = '\000';

    return (dbuf);
}

/*

Name:
	mkdir

Function:
	Make directories.

Algorithm:
	Make work copies of file anme.  Scan for next level.  If level doesn't
	exist, call mkdir command to create it with default permissions and user
	as owner.  If requested to preserve, make original file onwer the owner
	of the directory, too.

Parameters:
	Pointer to file pathname.

Returns:
	1 = ok
	exit = not ok

Files and Programs:
	/bin/mkdir


*/
mkdir (name,owner)
char   *name;
int owner;
{
char name2[40];
    char       *cp;
    int     statbuf[18];
    int     pstat;

    copy (name, name2);

    cp = name2 + 1;					/* ignore starting / character */
    for (;;)
    {							/* forever loop */
							/* find next part of file name */
	for (; (*cp != '/') && (*cp != '\0'); cp++);
	if (*cp == '\0')
	    return (1);
	*cp = '\0';					/* terminate string */
	if (stat (name2, statbuf) < 0)
	{
    again: 
		pstat = fork();
		switch(pstat) {
		case 0: 				/* child */
		    execl (MKDIR, MKDIR, name2, 0);
		    printf ("tp:Can't  find 'mkdir'!\n");
		    exit (ENOEXEC);
		case -1: 				/* error */
		    sleep (10);
		    goto again;
		default: 				/* error */
		    wait (&pstat);
			if(flags&flp)
				chown(name,owner);
	    }						/* switch */
	}						/* if stat */
	*cp = '/';
	cp++;
    }							/* forever */
}							/* mkdir */
