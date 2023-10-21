#
#include "tp.h"
/*

Name:
	clrdir

Function:
	Clear directory.

Algorithm:
	Clear entire directory space, 2 characters at a time.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
clrdir ()
{
    register    j,
               *p;

    j = ndirent * (DIRSIZE / 2);
    p = &dir;
    do
	(*p++ = 0);
    while (--j);
    lastd = 0;
}

/*

Name:
	clrent

Function:
	Clear direcotry entry.

Algorithm:
	Clear directory entry space.  Then scan for and readjust pointer to last
	empty entry.

Parameters:
	Pointer to directory entry.

Returns:
	None.

Files and Programs:
	None.


*/
clrent (ptr)
{
    register   *p,
                j;

    p = ptr;
    j = DIRSIZE / 2;
    do
	*p++ = 0;
    while (--j);
    if (p == lastd)
	do
	{
	    if (--lastd < &dir)
	    {
		lastd = 0;
		return;
	    }
	}
	while (lastd -> d_namep == 0);
}
/*

Name:
	rddir

Function:
	Read directory into core.

Algorithm:
	Clear directory space.  Skip over boot block.  Read in and scan
	directory entries, creating checksum.  Put pathnames in core buffer
	and create abbreviated directory in core.  Check for valid checksum.
	Create bitmap of used entries.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
rddir ()
{
    register   *p1,
               *tp,
                reg;
    struct dent *dptr;
    struct tent *tptr;
    int     count,
            sum;

    sum = 0;
    clrdir ();
    rseek (0);
    tread ();						/* Read the bootstrap block */
    if (((reg = tpentry[7].cksum) != 0) && (flags & flm))
    {
	ndirent = reg;
	ndentd8 = ndirent >> 3;
    }
    dptr = &dir[0];
    count = ndirent;
    do
    {
	if ((count & 07) == 0)
	{						/* next block */
	    tread ();
	    tptr = &tpentry[0];
	}
	tp = tptr;
	p1 = tp + (sizeof tpentry[0]) / 2;
	reg = 0;
	do
	    reg =+ *tp++;
	while (tp < p1);
	sum =| reg;
	p1 = dptr;
	if (reg == 0)
	{
	    tp = tptr;
	    if (*tp != 0)
	    {
		lastd = p1;
		encode (tp, p1);
		tp = &(tp -> mode);
		++p1;					/* skip namep */
		reg = (sizeof dir[0] / 2) - 1;
		do
		    *p1++ = *tp++;
		while (--reg);
	    }
	}
	++tptr;						/* bump to next tent */
	reg = dptr;
	reg -> d_mode =& ~0100000;			/* mark previous */
	dptr = (reg =+ sizeof dir[0]);
    }
    while (--count);
    if (sum != 0)
    {
	printf ("Directory checksum\n");
	if ((flags & fli) == 0)
	    done (EOTHER);
    }
    bitmap ();
}

/*

Name:
	wrdir

Function:
	Write direcotry from core.

Algorithm:
	Write boot block (or dummy) to tape.  For each block in the
	direcotry, expand the core version for writing to tape.  Get core
	pathname for the tape version.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
wrdir ()
{
    register    reg,
               *tp,
               *dp;
    struct dent *dptr;
    struct tent *tptr;
    int     count;

    wseek (0);
	reg = open (boot, 0);
    if (reg >= 0)
    {
	read (reg, tapeb, 512);
	close (reg);
	tpentry[7].cksum = ndirent;
    }
	else
		printf("%s: boot block unavailable\n",boot);
    dptr = &dir[0];
    count = ndirent;
    for (;;)
    {
	twrite ();		/* the boot block is written the first time */
	if (count == 0)
	    return;
	tp = &tpentry[0];
	do
	{
	    tptr = tp;
	    dp = dptr++;				/* dptr set to next entry */
	    if (dp -> d_namep)
	    {
		decode (tp, dp);
		tp = &(tp -> mode);			/* point to mode  */
		++dp;					/* point to d_mode */
		do
		    *tp++ = *dp++;
		while (dp < dptr);
		tp = tptr;
		dp = &tp[31];				/* points to checksum */
		reg = 0;
		do
		    reg =- *tp++;			/* form checksum */
		while (tp < dp);
		*tp++ = reg;
	    }
	    else
	    {
		reg = sizeof tpentry[0] / 2;
		do
		    *tp++ = 0;				/* clear entry */
		while (--reg);
	    }
	}
	while (--count & 07);
    }
}
/*

Name:
	tread

Function:
	Read block from tape.

Algorithm:
	Read block from tape and adjust block counter.  Report error, quit if
	flag not set to ignore.  Otherwise, clear block buffer.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
tread ()
{
    register    j,
               *ptr;

    if (read (fio, tapeb, 512) != 512)
    {
	printf ("Tape read error\n");
	if ((flags & fli) == 0)
	    done (EIO);
	ptr = tapeb;
	j = 256;
	do
	    *ptr = 0;
	while (--j);
    }
    rseeka++;
}

/*

Name:
	twrite

Function:
	Write block to tape.

Algorithm:
	Write block to tape and adjust block counter.  Report any errors and
	quit.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
twrite ()
{
    if (write (fio, tapeb, 512) != 512)
    {
	printf ("Tape write error\n");
	done (EIO);
    }
    ++wseeka;
}

/*

Name:
	rseek

Function:
	Seek to tape block for reading.

Algorithm:
	Seek to block on tape for reading.  Report any error.

Parameters:
	Block number.

Returns:
	None.

Files and Programs:
	None.


*/
rseek (blk)
{
    rseeka = blk;
    if (seek (fio, blk, 3) < 0)
	seekerr ();
}

/*

Name:
	wseek

Function:
	See to tape block for writing.

Algorithm:
	Seek to block on tape for writing.  If over 25 blocks away,
	seek to 1 block previous, and razead to get into position.

Parameters:
	Block number.

Returns:
	None.

Files and Programs:
	None.


*/
wseek (blk)
{
    register    amt,
                b;

    amt = b = blk;
    if ((amt =- wseeka) < 0)
	amt = -amt;
    if (amt > 25 && b)
    {
	seek (fio, b - 1, 3);				/* seek previous block */
	read (fio, &wseeka, 1);				/* read next block */
    }
    wseeka = b;
    if (seek (fio, b, 3) < 0)
	seekerr ();
}

/*

Name:
	seekerr

Function:
	Report seek error.

Algorithm:
	Print error message and quit.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
seekerr ()
{
    printf ("Tape seek error\n");
    done (EIO);
}
/*

Name:
	verify

Function:
	Verify action interactively.

Algorithm:
	If no verification requested, return.
	For the current file name, display action.  Return if appropriate.
	Otherwise, exit.

Parameters:
	Command name.

Returns:
	0 = action ok
	-1 = just carriage return entered.
	exit = action not ok

Files and Programs:
	None.


*/
verify (key)
{
    register    c;

    if ((flags & (flw | flv)) == 0)
	return (0);
repeat: 
    printf ("%c %s ", key, name);
    if ((flags & flw) == 0)
    {
	printf ("\n");
	return (0);
    }
    c = getchar ();
    if (c == 'n' && getchar () == '\n')
	done ();
    if (c == '\n')
	return (-1);
    if (c == 'y' && getchar () == '\n')
	return (0);
    while (getchar () != '\n');
    goto repeat;
}

/*

Name:
	getfiles

Function:
	Process file names from command line.

Algorithm:
	If no names, use ".".  For each command line filename, output its
	contents to tape.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
getfiles ()
{
    register char  *ptr1,
                   *ptr2;

    if (narg == 0)
    {
	name -> integer = '.\0';
	callout ();
    }
    else
	while (--narg >= 0)
	{
	    ptr1 = *parg++;
	    ptr2 = &name[0];
	    while (*ptr2++ = *ptr1++);
	    callout ();
	}
}
/*

Name:
	expand

Function:
	Expand directory pathname.

Algorithm:
	Open direcoory and read entries.  Report errors in doing either
	action.  Skip null entries, ".", and "..".  Append name and "/"
	to directory name in buffer.  Use "callout()" to output file contents or 
	to recurse if this was a directory.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
expand ()
{
    register char  *p0,
                   *p1,
                   *save0;
    int     n,
            fid;

    if ((fid = open (name, 0)) < 0)
	fserr ("tp.expand open");
    for (;;)
    {
	if ((n = read (fid, catlb, 16)) != 16)
	{
	    if (n == 0)
	    {
		close (fid);
		return;
	    }
	    fserr ("tp.expand read");
	}
	if (catlb[0] == 0)				/* null entry */
	    continue;
	p0 = &name[0];
	p1 = &catlb[1];
	if (equal (p1, ".") || equal (p1, ".."))
	    continue;
	while (*p0 != '\0') p0++;
	save0 = p0;					/* save loc of \0 */
	if (p0[-1] != '/')
	    *p0++ = '/';
	while (*p0++ = *p1++);
	callout ();
	*save0 = 0;					/* restore */
    }
}

/*

Name:
	fserr

Function:
	Report file access error.

Algorithm:
	Print message and exit.

Parameters:
	Pointer to message.

Returns:
	None.

Files and Programs:
	None.


*/
fserr (str)
char *str;
{
    printf ("%s: Cannot open file - %s\n", str, name);
    done (ENOENT);
}

/*

Name:
	callout

Function:
	Output file.

Algorithm:
	Stat the constructed pathname in the name buffer.  If it's a directory,
	expand to the next level.  Return if it's a special file.  Scan core
	directory for empty slot and same pathname.  
	If found:
	If updating, check times and return if you already have the newest
	version on tape.  Fill in the core directory entry with the new size,
	time, and owner info.  Mark it as a new entry.
	If not found:
	Try to use empty or next available slot.  Report if directory is full.
	Put file pathname in core buffer and update info detailed above.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
callout ()
{
    register struct dent   *d;
    register char  *ptr1,
                   *ptr0;
    char   *empty;

    if (stat (name, &statb) < 0)
	fserr ("tp.callout");
    ptr0 = statb.s_flags;
    if ((ptr0 =& 060000) != 0)
    {
	if (ptr0 == 040000)				/* directory */
	    expand ();
	return;
    }
							/* when we reach here we have recursed until we found * an
							   ordinary file.  Now we look for it in "dir". */
    empty = 0;
    d = &dir[0];
    do
    {
	if (d -> d_namep == 0)
	{						/* empty directory slot */
	    if (empty == 0)				/* remember the first one */
		empty = d;
	    continue;
	}
	decode (name1, d);
	ptr0 = &name[0];
	ptr1 = &name1;
	do
	    if (*ptr0++ != *ptr1)
		goto cont;
	while (*ptr1++);
							/* veritably the same name */
	if (flags & flu)
	{						/* check the times */
	    if (d -> d_time[0] > statb.s_modtime[0])
		return;
	    if (d -> d_time[0] == statb.s_modtime[0] &&
		    d -> d_time[1] >= statb.s_modtime[1])
		return;
	}
	if (verify ('r') < 0)
	    return;
	goto copydir;
cont: 
	continue;
    }
    while (++d <= lastd);
							/* name not found in directory */
    if ((d = empty) == 0)
    {
	d = lastd + 1;
	if (d >= edir)
	{
	    printf ("Directory overflow\n");
	    done (EOTHER);
	}
    }
    if (verify ('a') < 0)
	return;
    if (d > lastd)
	lastd = d;
    encode (name, d);
copydir: 
    d -> d_mode = statb.s_flags | 0100000;
    d -> d_uid = statb.s_uid;
    d -> d_gid = statb.s_gid;
    if (flags & flf)
    {							/* fake entry */
	statb.s_size0 = 0;
	statb.s_size1 = 0;
    }
    d -> d_size0 = statb.s_size0;
    d -> d_size1 = statb.s_size1;
    d -> d_time[0] = statb.s_modtime[0];
    d -> d_time[1] = statb.s_modtime[1];
}
