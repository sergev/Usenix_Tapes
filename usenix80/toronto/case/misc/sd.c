#
/*
 *	sd - search dir
 *	
 *	give information about directories
 *	
 *	Greg Ordy - Case Western Reserve University
 *
 *	options:
 *		-r:	recursive
 *		-i:	print inode number
 *		-w:	total # of unused directory slots
 *		-o:	list names of files found in dir.
 *		-a:	include files and directories beginning
 *			with "."
 *		-d: 	list directories only
 *
*/

#define		DEFAULT		"."			/* search . if no args */
#define		OK		3
#define		BAD		4
#define		OFFSET		4

struct dir_entry
{
    int     inumb;
    char    name[14];
};

struct inode
{
    char    minor;
    char    major;
    int     inumber;
    int     flags;
    char    nlinks;
    char    uid;
    char    gid;
    char    size0;
    int     size1;
    int     addr[8];
    int     actime[2];
    int     modtime[2];
};

int     rflg, iflg, aflg, wflg, oflg, dflg;
int     level, waste;

main (argc, argv)
int     argc;
char   *argv[];
{
    register int    i, j;

    close (0);
    close (2);

    for (j = 1; j < argc && argv[j][0] == '-'; j++)
	for (i = 1; argv[j][i]; i++)
	    switch (argv[j][i])
	    {
		case 'r': 
		    rflg++;
		    break;
		case 'i': 
		    iflg++;
		    break;
		case 'a': 
		    aflg++;
		    break;
		case 'w': 
		    wflg++;
		    break;
		case 'o': 
		    oflg++;
		    break;
		case 'd':
		    dflg++;
		    break;
		default: 
		    printf ("Unknown option: %c\n",
			    argv[j][i]);
		    break;
	    }
    if (j == argc)
    {
	find (DEFAULT);
	if (wflg)
	    printf ("Empty entries: %d\n", waste);
	exit (0);
    }

    for (; j != argc; j++)
    {
	if (chdir (argv[j]) < 0)
	{
	    printf ("%s: cannot examine\n", argv[j]);
	    continue;
	}
	printf ("%s:\n", argv[j]);
	find (argv[j]);
	putchar ('\n');
    }
    if (wflg)
	printf ("Empty entries: %d\n", waste);
}

find (dir)
char   *dir;
{
    int     fd;
    register int    i, j;
    struct dir_entry    entry;

    if ((fd = open (dir, 0)) < 0)
    {
	printf ("%s: cannot open\n", dir);
	return;
    }

    if (aflg)
    {
	for (j = 0; j != 2; j++)
	{
	    read (fd, &entry, sizeof entry);
	    for (i = 0; i != level; i++)
		putchar (' ');
	    printf ("%s", entry.name);
	    if (iflg)
	    {
		for (i = 0; i != 14; i++)
		    if (entry.name[i] == 0)
			putchar (' ');
		putchar (' ');
		printf (" %4d", entry.inumb);
	    }

	    putchar ('\n');
	}
    }
    else
	seek (fd, 32, 0);
    while (read (fd, &entry, sizeof entry) == sizeof entry)
    {
	if (entry.inumb == 0)
	{
	    waste =+ 1;
	    if (oflg)
		printf ("*** %s\n", entry.name);
	    continue;
	}
	if (entry.name[0] == '.' && aflg == 0)
	    continue;
	if (dflg)
	    if(check (entry.name) != OK)
		continue;
	for (i = 0; i != level; i++)
	    putchar (' ');
	printf ("%s", entry.name);
	if (iflg)
	{
	    for (i = 0; i != 14; i++)
		if (entry.name[i] == 0)
		    putchar (' ');
	    putchar (' ');
	    printf (" %4d", entry.inumb);
	}
	putchar ('\n');
	if (rflg)
	{
	    if (check (entry.name) != OK)
		continue;
	    if (chdir (entry.name) < 0)
	    {
		printf ("%s: cannot examine\n", entry.name);
		continue;
	    }
	    level =+ OFFSET;
	    find (".");
	    level =- OFFSET;
	    chdir ("..");
	}
    }
    close (fd);
}

check (dir)
char   *dir;
{
    struct inode    ibuf;
    if (stat (dir, &ibuf) == -1)
    {
	printf ("%s: cannot stat\n", dir);
	return (BAD);
    }
    if ((ibuf.flags & 040000) != 040000)
	return (BAD);
    return (OK);
}
