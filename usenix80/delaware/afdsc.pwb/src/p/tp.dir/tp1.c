#include "tp.h"
/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:

Installation Instructions:

History:
	c-version of tp?.s
 	M. Ferentz
 	August 1976
       2.1 dlm  14 Dec 1977
       Change of format of tv option to allow the printing of
       information about large sized files and large block
       numbers
 *      2.2 dlm  12 Jun 1978
 *      Change to have non-existent directories madee if not already.

 */

char   *prog_id "~|^' tp.c  Release 3 Version 7\n";
int     flags ;					/* default is flu */

main (argc, argv)
char  **argv;
{
    register char   c;
    extern  cmd (), cmr (), cmx (), cmt ();

    nextarg;
    command = &cmr;
	flags = flu|flm;	/* set default  for update and mag tape */
	boot = MBOOT;
    if (argc < 1) exit (EFLAG);
    while (argc > 0 && **argv == '-')
    {
	for (;;)
	{
	    switch (*++*argv)
	    {
		case '0': 
		case '1': 
		case '2': 
		case '3': 
		case '4': 
		case '5': 
		case '6': 
		case '7': 
		    tc[8] = c;
		    mt[7] = c;
		    continue;
		case 'c': 
		    flags =| flc;
			boot = TBOOT;
		    continue;
		case 'd': 
		    setcom (&cmd);
		    continue;
		case 'f': 
		    flags =| flf;
		    continue;
		case 'i': 
		    flags =| fli;
		    continue;
		case 'm': 
		    flags =| flm;
			boot = MBOOT;
		    continue;
		case 'r': 
		    flags =& ~flu;
		    setcom (&cmr);
		    continue;
		case 't': 
		    setcom (&cmt);
		    continue;
		case 'u': 
		    flags =| flu;
		    setcom (&cmr);
		    continue;
		case 'v': 
		    flags =| flv;
		    continue;
		case 'w': 
		    flags =| flw;
		    continue;
		case 'x': 
		    setcom (&cmx);
		    continue;
		case 'p':
			flags =| flp;
			continue;
		case 'o':
			flags =| flo;
			setcom(&cmx);
			continue;
		case 'b':
			nextarg;
			boot = *argv;
			goto ugh_a_goto;
		case '\0':
		    goto ugh_a_goto;
		default: 
		    exit (EFLAG);
	    }
	}
ugh_a_goto: 
	nextarg;
    }
    narg = rnarg = argc;
    parg = argv;
    optap ();
    nptr = nameblk = sbrk (0);
    top = sbrk (512);
    (*command) ();
    exit (ENOERR);
}

/*

Name:
	optap

Function:
	Open tape drive.

Algorithm:
	Open either mag or DEC tape.  Set appropriate size limits for media.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	/dev/mt0
	/dev/tapx


*/
optap ()
{
    extern  cmr ();

    if ((flags & flm) == 0)
    {							/*  DECTAPE */
	tapsiz = 578;
	ndirent = 192;
	fio = open (tc, 2);
    }
    else
    {							/* MAGTAPE */
	tapsiz = 32767;
	ndirent = MDIRENT;
	if (command == &cmr)
	    fio = open (mt, 1);
	else
	    fio = open (mt, 0);
    }
    if (fio < 0)
    {
	printf ("tp: No such file or directory - %s\n", mt);
	exit (ENOENT);
    }
    ndentd8 = ndirent >> 3;
    edir = &dir[ndirent];
}

/*

Name:
	setcom

Function:
	Set command pointer for operation to be performed.

Algorithm:
	If command has already been sspecified, give error.
	Set address of new command.

Parameters:
	Command to be executed.

Returns:
	None.

Files and Programs:
	None.


*/
setcom (newcom)
{
    extern  cmr ();

    if (command != &cmr)
	useerr ();
    command = newcom;
}

/*

Name:
	useerr

Function:
	Print user error message.

Algorithm:
	Print error message and exit.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
useerr ()
{
    printf ("Bad usage\n");
    exit (EFLAG);
}
/*

Name:
	cmd

Function:
	pre-process delete command.

Algorithm:
	Check for illegal flag combination.  Read in directory.  Delete entry.
	Write out directory.  Print usage stats.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
cmd ()
{
    extern  delete ();

    if (flags & (flm | flc | flf))
	useerr ();
    if (narg <= 0)
	useerr ();
    rddir ();
    gettape (&delete);
    wrdir ();
    check ();
}

/*

Name:
	cmr

Function:
	Pre-process replace command.

Algorithm:
	Check for illegal flag combinations.  Clear mag tape directory or
	read DEC tape directory.  Gather files and write them to tape.
	Print usage stats.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
cmr ()
{
    if (flags & (flc | flm))
	clrdir ();
    else
	rddir ();
    getfiles ();
    update ();
    check ();
}

/*

Name:
	cmt

Function:
	Pre-process table of contents command.

Algorithm:
	Check for illegal flag combinations.  Read directory, print header,
	and print file information.
	Print usage stats.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
cmt ()
{
    extern  taboc ();

    if (flags & (flc | flf | flw))
	useerr ();
    rddir ();
    if (flags & flv)
	printf ("   mode    uid gid tapa    size     date    time   name\n");
    gettape (&taboc);
    check ();
}

/*

Name:
	cmx

Function:
	Pre-process extract command.

Algorithm:
	Check for illegal flag combinations.  Read in directory, read individual
	file(s) and print ending message.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
cmx ()
{
    extern  extract ();

    if (flags & (flc | flf))
	useerr ();
    rddir ();
    gettape (&extract);
    done (ENOERR);
}

/*

Name:
	check

Function:
	Check for resource usage.

Algorithm:
	Print usage messages and exit.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
check ()
{
    usage ();
    done (ENOERR);
}

/*

Name:
	done

Function:
	Print ending message

Algorithm:
	Print message and exit.

Parameters:
	Exit code.

Returns:
	None.

Files and Programs:
	None.


*/
done (cond)
{
	if(flags&flo)
		write(2,"End\n",4);
	else
		printf("End\n");
	exit (cond);
}

/*

Name:
	encode

Function:
	Copy file name into core buffer.

Algorithm:
	Copy file path name into core buffer and save pointer.
	Check for core and pathname limts and tell user of errors.

Parameters:
	Pointer to pathname.
	Pointer to directory entry.

Returns:
	None.

Files and Programs:
	None.


*/
encode (pname, dptr)					/* pname points to the pathname * nptr points to next location
							   in nameblk * dptr points to the dir entry		   */
{
    register char  *np,
                   *sp;

    sp = pname;
    dptr -> d_namep = np = nptr;
    if (np > top - 32)
    {
	if ((top = sbrk (512)) < 0)
	{
	    printf ("Out of core\n");
	    done (ENOMEM);
	}
    }
    while (*np++ = *sp++);
    if (np <= nptr + 32)
	nptr = np;
    else
    {
	printf ("Pathname too long - %s\nFile ignored\n", nptr);
	clrent (dptr);
    }
}

/*

Name:
	decode

Function:
	Copy file name from core buffer.

Algorithm:
	Copy file pathname from core buffer.

Parameters:
	Pointer to receiving buffer.
	Pointer to directory entry.

Returns:
	None.

Files and Programs:
	None.


*/
decode (pname, dptr)					/* dptr points to the dir entry * name is placed in pname[] */
{

    register char  *p1,
                   *p2;

    p1 = pname;
    p2 = dptr -> d_namep;
    while (*p1++ = *p2++);
}
