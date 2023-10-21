/*
 * Lar - LAR format library file maintainer
 * by Stephen C. Hemminger
 *	linus!sch	or	sch@Mitre-Bedford
 *
 * Heavily hacked for MS-DOS by Eric C. Brown
 *      utah-cs!brownc	or	brownc@utah-cs
 *
 *  Usage: lar key library [files] ...
 *
 *  Key functions are:
 *	u - Update, add files to library
 *	t - Table of contents
 *	e - Extract files from library
 *      a - extract All files from library
 *	p - Print files in library
 *	d - Delete files in library
 *	r - Reorganize library
 *  Other keys:
 *	v - Verbose
 *
 *  This program is public domain software, no warranty intended or
 *  implied.
 *
 *  DESCRPTION
 *	Lar is a MS-DOS program to manipulate program libraries.
 *	The primary use of lar is to combine several files together
 *	to reduce disk space used.
 *	Lar maintains the date and time of files in the library, so
 *	that Make can extract the date/time for rebuilding a file.
 *	When files are restored, the file has the original date/time
 *	rather than the date/time when the file was extracted.
 *     The original CP/M library program LU is the product
 *     of Gary P. Novosielski. 
 *
 *  PORTABILITY
 *     The code is modeled after the Software tools archive program,
 *     and is setup for Version 7 Unix.  It does not make any assumptions
 *     about byte ordering, explict and's and shift's are used.
 *     If you have a dumber C compiler, you may have to recode new features
 *     like structure assignment, typedef's and enumerated types.
 *
 *  * Unix is a trademark of Bell Labs.
 *  ** CP/M is a trademark of Digital Research.
 */
#include "lar.h"

/* Globals */
char   *fname[MAXFILES];
bool ftouched[MAXFILES];

struct ludir ldir[MAXFILES];
int     errcnt, nfiles, nslots;
bool	verbose = false;

char   *getname(), *malloc(), *fgets();
long	fcopy(), lseek(), rlwtol();
int	update(), reorg(), table(), extract(), print(), delete(), getall();

typedef int (*PFI)();

main (argc, argv)
int	argc;
char  **argv;
{
	register char *flagp;
	char   *aname;			/* name of library file */
	PFI function = (PFI) NULL;	/* function to do on library */
	/* set the function to be performed, but detect conflicts */
#define setfunc(val)	if(function != (PFI) NULL) conflict(); else function = val

	if (argc < 3)
	help ();

	aname = argv[2];
	filenames (argc, argv);

	for(flagp = argv[1]; *flagp; flagp++)
	switch (*flagp) {
		case 'u': 
		    setfunc(update);
		    break;
		case 't': 
		    setfunc(table);
		    break;
		case 'e': 
		    setfunc(extract);
		    break;
		case 'a': 
		    setfunc(getall);
		    break;
		case 'p': 
		    setfunc(print);
		    break;
		case 'd': 
		    setfunc(delete);
		    break;
		case 'r': 
		    setfunc(reorg);
		    break;
		case 'v':
		    verbose = true;
		    break;
		default: 
		    help ();
		}

	if(function == (PFI) NULL) {
	   fputs("No function key letter specified\n", stderr);
	   help();
	}

	(*function)(aname);
	exit(0);
}

/* print error message and exit */
help () {
fputs ("Usage: lar {uteapdr}[v] library [files] ...\n", stderr);
fputs ("Functions are:\n\tu - Update, add files to library\n", stderr);
fputs ("\tt - Table of contents\n", stderr);
fputs ("\te - Extract files from library\n", stderr);
fputs ("\ta - extract All files from library\n", stderr);
fputs ("\tp - Print files in library\n", stderr);
fputs ("\td - Delete files in library\n", stderr);
fputs ("\tr - Reorganize library\n", stderr);

fputs ("Flags are:\n\tv - Verbose\n", stderr);
exit (1);
}

conflict() {
fputs ("Conficting keys\n", stderr);
help();
}

/*
 * This ltoa doesn't call in the floating point library:
 *   CI C86 does an sprintf!
 */
ltoa(val, buf)
long val;
register char *buf;
{
	register int i;
	int j;
	char tbuf[20];

	if (val == 0) {
		buf[0] = '0';
		buf[1] = '\0';
		return;
	}

	i = 19;
	while (val != 0) {
		tbuf[i--] = (val % 10) + '0';
		val /= 10;
	}

	i++;
	for (j = 0; i <= 19; i++, j++)
		buf[j] = tbuf[i];

	buf[j] = '\0';
}

/* Get file names, check for dups, and initialize */
filenames (ac, av)
int ac;
char  **av;
{
	register int i, j=0;
	int k, loop;
	struct ffblk ff;
	bool iswild();

	errcnt = 0;
	for (i = 0; i < ac - 3; i++) {
	    if (iswild(av[i + 3])) {
	        k = findfirst(av[i + 3], &ff, 0);
		while (k == 0) {
	            fname[j] = malloc(strlen(ff.ff_name)+1);
	            if (fname[j] == NULL)
                        error("Out of core..");
		    strcpy(fname[j], ff.ff_name);
		    for (loop = 0; ff.ff_name[loop] != 0; loop++)
		        fname[j][loop] = tolower(ff.ff_name[loop]);
	            ftouched[j] = false;
	            if (j == MAXFILES)
	                error ("Too many file names.");
	            j++;
	            k = findnext(&ff);
	        }				/* while */
	    }				/* if-then */
	    else {		/* not a wildcard */
	        fname[j] = av[i+3];
	        ftouched[j] = false;
	        if (j == MAXFILES)
	            error ("Too many file names.");
	        j++;
	    }				/* else */
	}					/* for */
	checkdups(j);
}

bool iswild(str)
register char *str;
{
	while (*str != '\0') {
            if (*str == '*' || *str == '?')
                return true;
            else str++;
	}
	return false;
}

checkdups(i)
register int i;
{
	register int j;

	fname[i] = NULL;
	nfiles = i;
	for (i = 0; i < nfiles; i++)
            for (j = i + 1; j < nfiles; j++)
	        if (equal (fname[i], fname[j])) {
	            fputs (fname[i], stderr);
	            error (": duplicate file name");
                }
}	

table (lib)
char   *lib;
{
	fildesc lfd;
	register int i;
	register struct ludir *lptr;
	long total, offset, size;
	int active = 0, unused = 0, deleted = 0, k;
	char *uname, buf[20];

	if ((lfd = _open (lib, O_RDWR)) == ERROR)
	    cant (lib);

	getdir (lfd);
	total = lwtol(ldir[0].l_len);
	if (verbose) {
	    puts("Name             Index          Length");
	   fputs("Directory                       ", stdout);
	    ltoa(total, buf);
	    puts(buf);
	}

	for (i = 1, lptr = &ldir[1]; i < nslots; i++,lptr++)
	    switch(lptr->l_stat) {
		case ACTIVE:
	            active++;
 		    uname = getname(lptr->l_name, lptr->l_ext);
	            if (filarg (uname))
            		if(verbose) {
		            offset = lwtol(lptr->l_off);
                	    size = lwtol(lptr->l_len);
			    fputs(uname, stdout);
			    for (k = 1; k < 18 - strlen(uname); k++)
			       fputc(' ', stdout);
			    ltoa(offset, buf);
			    fputs(buf, stdout);
			    for (k = 1; k < 16 - strlen(buf); k++)
			       fputc(' ', stdout);
			    ltoa(size, buf);
			    puts(buf, stdout);
    			}
 		        else
			   puts(uname);
		    total += lwtol(lptr->l_len);
		break;
	case UNUSED:
		unused++;
		break;
	default:
		deleted++;
	}
	if(verbose) {
	    puts("--------------------------------------");
	    fputs("Total bytes      ", stdout);
	    ltoa(total, buf);
	    puts(buf);
	    fputs("\nLibrary ", stdout);
	    fputs(lib, stdout);
	    fputs(" has ", stdout);
	    itoa(nslots, buf);
	    fputs(buf, stdout);
	    fputs(" slots, ", stdout);
	    itoa(deleted, buf);
	    fputs(buf, stdout);
	    fputs(" deleted ", stdout);
	    itoa(active, buf);
	    fputs(buf, stdout);
	    fputs(" active, ", stdout);
	    itoa(unused, buf);
	    fputs(buf, stdout);
	    puts(" unused");
          }

    VOID _close (lfd);
    not_found ();
}

putdir (f)
fildesc f;
{

    lseek(f, 0L, 0);		/* rewind f */
    if (_write (f, (char *) ldir, DSIZE * nslots) != nslots * DSIZE)
	error ("Can't write directory - library may be botched");
}

initdir (f)
fildesc f;
{
    register int    i;
    long    numbytes;
    char    line[80];
    static struct ludir blankentry = {
	UNUSED,
	{ ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },
	{ ' ', ' ', ' ' },
    };
    static struct ludir nameentry = {
	ACTIVE,
	{ 'l', 'a', 'r', 'f', 'o', 'r', 'm', 't' },
	{ 'a', 'r', 'c' },
    };

    for (;;) {
	fputs ("Number of slots to allocate (1-255): ", stdout);
	if (fgets (line, 80, stdin) == NULL)
	    error ("Eof when reading input");
	nslots = atoi (line);
	if (nslots < 1)
	    puts ("Must have at least one!");
	else if (nslots > MAXFILES)
	    puts ("Too many slots");
	else
	    break;
    }

    numbytes = nslots * DSIZE;

    for (i = 1; i < nslots; i++)
	ldir[i] = blankentry;
    ldir[0] = nameentry;
    ltolw (ldir[0].l_len, numbytes);
    ltolw (ldir[0].l_datetime, -1L);	/* make funny date field */

    putdir (f);
}

putname (cpmname, unixname)
char   *cpmname, *unixname;
{
    register char  *p1, *p2;

    for (p1 = unixname, p2 = cpmname; *p1; p1++, p2++) {
	while (*p1 == '.') {
	    p2 = cpmname + 8;
	    p1++;
	}
	if (p2 - cpmname < 11)
	    *p2 = islower(*p1) ? toupper(*p1) : *p1;
	else {
	    fputs (unixname, stderr);
	    fputs (": name truncated\n", stderr);
	    break;
	}
    }
    while (p2 - cpmname < 11)
	*p2++ = ' ';
}

/* filarg - check if name matches argument list */
filarg (name)
char   *name;
{
    register int    i;

    if (nfiles <= 0)
	return 1;

    for (i = 0; i < nfiles; i++)
	if (equal (name, fname[i])) {
	    ftouched[i] = true;
	    return 1;
	}

    return 0;
}

not_found () {
    register int    i;

    for (i = 0; i < nfiles; i++)
	if (!ftouched[i]) {
	    fputs(fname[i], stderr);
	    fputs(": not in library.\n", stderr);
	    errcnt++;
	}
}

extract(name)
char *name;
{
	getfiles(name, false);
}

print(name)
char *name;
{
	getfiles(name, true);
}

getall (libname)
char   *libname;
{
    fildesc lfd, ofd;
    register int    i;
    register struct ludir *lptr;
    union timer timeunion;
    extern int errno;
    char   *unixname;

    if ((lfd = _open (libname, O_RDWR)) == ERROR)
	cant (libname);

    getdir (lfd);

    for (i = 1, lptr = &ldir[1]; i < nslots; i++, lptr++) {
	if(lptr->l_stat != ACTIVE)
		continue;
	unixname = getname (lptr->l_name, lptr->l_ext);
	fputs(unixname, stderr);
	ofd = _creat(unixname, 0);
	if (ofd == ERROR) {
	    fputs ("  - can't create", stderr);
	    errcnt++;
	}
	else {
	    VOID lseek (lfd, (long) lwtol (lptr->l_off), 0);
	    acopy (lfd, ofd, lwtol (lptr->l_len));
	    timeunion.realtime = lwtol(lptr->l_datetime);
            VOID setftime(ofd, &(timeunion.ftimep));
	    VOID _close (ofd);
	}
	putc('\n', stderr);
    }
    VOID _close (lfd);
    not_found ();
}

getfiles (name, pflag)
char   *name;
bool	pflag;
{
    fildesc lfd, ofd;
    register int    i;
    register struct ludir *lptr;
    union timer timeunion;
    extern int errno;
    char   *unixname;

    if ((lfd = _open (name, O_RDWR)) == ERROR)
	cant (name);

    ofd = pflag ? fileno(stdout) : ERROR;
    getdir (lfd);

    for (i = 1, lptr = &ldir[1]; i < nslots; i++, lptr++) {
	if(lptr->l_stat != ACTIVE)
		continue;
	unixname = getname (lptr->l_name, lptr->l_ext);
	if (!filarg (unixname))
	    continue;
	fputs(unixname, stderr);
	if (ofd != fileno(stdout))
	    ofd = _creat(unixname, 0);
	if (ofd == ERROR) {
	    fputs ("  - can't create", stderr);
	    errcnt++;
	}
	else {
	    VOID lseek (lfd, (long) lwtol (lptr->l_off), 0);
	    acopy (lfd, ofd, lwtol (lptr->l_len));
	    timeunion.realtime = lwtol(lptr->l_datetime);
	    if (ofd != fileno(stdout)) {
                VOID setftime(ofd, &(timeunion.ftimep));
		VOID _close (ofd);
	    }
	}
	putc('\n', stderr);
    }
    VOID _close (lfd);
    not_found ();
}

acopy (fdi, fdo, nbytes)		/* copy nbytes from fdi to fdo */
fildesc fdi, fdo;
long nbytes;
{
    register int btr, retval;
    char blockbuf[BLOCK];
    
    for (btr = (nbytes > BLOCK) ? BLOCK : (int) nbytes; btr > 0; 
    	 nbytes -= BLOCK, btr = (nbytes > BLOCK) ? BLOCK : (int) nbytes)  {
        if ((retval = _read(fdi, blockbuf, btr)) != btr) {
	    if( retval == 0 ) {
		error("Premature EOF\n");
	     }
	     if( retval == ERROR)
	        error ("Can't read");
	}
	if ((retval = _write(fdo, blockbuf, btr)) != btr) {
	     if( retval == ERROR )
	        error ("Write Error");
	}
    }
}

update (name)
char   *name;
{
    fildesc lfd;
    register int    i;

    if ((lfd = _open(name, O_RDWR)) == ERROR) {
	if ((lfd = _creat (name, 0)) == ERROR) {
	    cant (name);
	}
	else 
	    initdir (lfd);
    }
    else
	getdir (lfd);		/* read old directory */

    if(verbose)
	    fputs ("Updating files:\n", stderr);
    for (i = 0; i < nfiles; i++)
	addfil (fname[i], lfd);
    if (errcnt == 0)
	putdir (lfd);
    else
	fputs("fatal errors - library not changed\n", stderr);
    VOID _close (lfd);
}

addfil (name, lfd)
char   *name;
fildesc lfd;
{
    fildesc ifd;
    register int i;
    register struct ludir *lptr;
    long byteoffs, numbytes;
    union timer timeunion;

    if ((ifd = _open(name, O_RDWR)) == ERROR) {
        fputs("LAR: can't find ", stderr);
	fputs(name, stderr);
	fputs(" to add\n", stderr);
	errcnt++;
	return;
    }
    if(verbose) {
	fputs(name, stderr);
	fputs("\n", stderr);
    }
    for (i = 1, lptr = ldir+1; i < nslots; i++, lptr++) {
	if (equal( getname (lptr->l_name, lptr->l_ext), name) ) /* update */
	    break;
	if (lptr->l_stat != ACTIVE)
		break;
    }
    if (i >= nslots) {
	fputs(name, stderr);
	fputs(": can't add library is full\n",stderr);
	errcnt++;
	return;
    }

    lptr->l_stat = ACTIVE;
    putname (lptr->l_name, name);
    byteoffs = lseek(lfd, 0L, 2);	/* append to end; return byte offset */

    ltolw (lptr->l_off, byteoffs);
    numbytes = fcopy (ifd, lfd);
    ltolw (lptr->l_len, numbytes);
    getftime(ifd, &(timeunion.ftimep));
    ltolw (lptr->l_datetime, timeunion.realtime);
    VOID _close (ifd);
}

long fcopy (ifd, ofd)		/*  copy file ifd (file) to ofd (library) */
fildesc ifd, ofd;
{
    long total = 0L;
    register int n;
    char blockbuf[BLOCK];

    while ( (n = _read(ifd, blockbuf, BLOCK)) > 0) {
	if (_write(ofd, blockbuf, n) != n)
		error("write error");
	total += (long) n;
    }
    return total;
}

delete (lname)
char   *lname;
{
    fildesc f;
    register int    i;
    register struct ludir *lptr;

    if ((f = _open(lname, O_RDWR)) == ERROR)
	cant (lname);

    if (nfiles <= 0)
	error("delete by name only");

    getdir (f);
    for (i = 0, lptr = ldir; i < nslots; i++, lptr++) {
	if (!filarg ( getname (lptr->l_name, lptr->l_ext)))
	    continue;
	lptr->l_stat = DELETED;
    }

    not_found();
    if (errcnt > 0)
	fputs ("errors - library not updated\n", stderr);
    else
	putdir (f);
    VOID _close (f);
}

reorg (name)
char  *name;
{
    fildesc olib, nlib;
    int oldsize, k;
    register struct ludir *optr;
    register int i, j;
    struct ludir odir[MAXFILES], *nptr;
    char tmpname[80], buf[10];
    char *mktemp();

    strcpy(tmpname, mktemp("libXXXXXX"));

    if( (olib = _open(name, O_RDWR)) == ERROR)
	cant(name);

    if( (nlib = _creat(tmpname, 0)) == ERROR)
	cant(tmpname);

    getdir(olib);
    fputs("Old library has ", stdout);
    itoa(oldsize = nslots, buf);
    fputs(buf, stdout);
    puts(" slots.");
/* copy ldir into odir */
    for(i = 0, optr = ldir, nptr = odir ; i < nslots ; i++, optr++, nptr++)
	movmem((char *) optr, (char *) nptr, sizeof(struct ludir));
/* reinit ldir */
    initdir(nlib);
    errcnt = 0;

/* copy odir's files into ldir */
    for (i = j = 1, optr = odir+1; i < oldsize; i++, optr++) {
	if( optr->l_stat == ACTIVE ) {
	    if(verbose) {
	        fputs("Copying: ", stderr);
		for (k = 0; k < 8; k++) 
		    fputc(optr->l_name[k], stderr);
		fputc('.', stderr);
		for (k = 0; k < 3; k++)
		    fputc(optr->l_ext[k], stderr);
		fputc('\n', stderr);
	    }
	    copyentry( optr, olib,  &ldir[j], nlib);
	    if (++j >= nslots) {
		errcnt++;
		fputs("Not enough room in new library\n", stderr);
		break;
	    }
        }
    }

    VOID _close(olib);
    putdir(nlib);
    VOID _close (nlib);

    if(errcnt == 0) {
	if (unlink(name) < 0 || rename(tmpname, name) < 0) {
	    cant(name);
	    exit(1);
	}
    }
    else
	fputs("Errors, library not updated\n", stderr);
    VOID unlink(tmpname);
}

copyentry( old, of, new, nf )
struct ludir *old, *new;
fildesc of, nf;
{
    long byteoffs, numbytes;
    register int btr;
    char blockbuf[BLOCK];
    
    new->l_stat = ACTIVE;
    movmem(old->l_name, new->l_name, 8);		/* copy name */
    movmem(old->l_ext, new->l_ext, 3);			/* copy extension */
    numbytes = lwtol(old->l_datetime);			/* copy date & time */
    ltolw(new->l_datetime, numbytes);
    VOID lseek(of, (long) lwtol(old->l_off), 0);
    byteoffs = lseek(nf, 0L, 2);	/* append to end; return new pos. */

    ltolw (new->l_off, byteoffs);
    numbytes = lwtol(old->l_len);
    ltolw (new->l_len, numbytes);

    for (btr = (numbytes > BLOCK) ? BLOCK : (int) numbytes; btr > 0; 
    	 numbytes -= BLOCK, btr = (numbytes > BLOCK) ? BLOCK : (int) numbytes
        )  {
        if (_read(of, blockbuf, btr) != btr) {
	    error ("Read Error in CopyEntry");
	}
	if (_write(nf, blockbuf, btr) != btr) {
	    error ("Write Error in CopyEntry");
	}
    }
}
