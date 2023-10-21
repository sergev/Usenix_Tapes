/*
 *	RT11 EMULATOR
 *	i/o routines
 *
 *	Daniel R. Strick
 *	May 31, 1979
 *	4/14/81 -- Changed ttyfil() to recognize tty devices with filenames
 *	7/17/81 -- added VAX condition to VIRTUAL option
 *	7/21/81 -- added delete() and rename() (untested!)
 *	8/30/81 -- savestatus now saves the file name rather than the descriptor
 */

#include "define.h"
#include "syscom.h"

char	libpref[]	= "/user/rtfiles/";
#define	NAMSTO	300	/* number of characters in file name storage */

wordp	comprtn;		/* address of completion routine */
word	compcsw;		/* channel status word for comp rout */
word	compchn;		/* I/O channel number for comp rout */


#define	SVSIZE	10		/* number of bytes stored by savestatus */

struct	channel	{
	int	ch_fd;		/* open file descriptor or #<0 */
	int	ch_err;		/* YES if I/O error during last xfer */
	struct	ssblok	{	/* name of open file (or pointer!?) */
		char	ssn[SVSIZE];
	}ch_ssb;
}
channel[CHNMAX] = {
	-1, NO, "", -1, NO, "", -1, NO, "", -1, NO, "",
	-1, NO, "", -1, NO, "", -1, NO, "", -1, NO, "",
	-1, NO, "", -1, NO, "", -1, NO, "", -1, NO, "",
	-1, NO, "", -1, NO, "", -1, NO, "", -1, NO, ""
};

char	fnstore[NAMSTO];	/* storage for names of open files */
char	*fnbreak = fnstore;	/* to free file name storage */


struct	devtab	{
	char	*dv_name;	/* device name */
	word	dv_status;	/* device type code and flags */
	word	dv_size;	/* device block size */
}
devtab[]	= {
	"dk",	0100000,	1,
	"sy",	0100000,	1,
	"tt",	      4,	0,
	"kb",	      4,	0,
	NULL
};


/*
 *  Return device status
 *  (device characteristics & handler size, address)
 */
dstatus (devblk, cblkp)
rad50 *devblk;
word *cblkp;
{
	register word *cp;
	register struct devtab *dp;
	char dname[4];

	fnschr (dname, devblk, 1);
	for (dp = devtab;  dp->dv_name != NULL;  ++dp)
		if (streq (dname, dp->dv_name)) {
			cp = cblkp;
			*cp++ = dp->dv_status;
			*cp++ = 040;	/* fake handler size */
			*cp++ = 013;	/* fake handler entry point */
			*cp++ = dp->dv_size;
			return;
		}
	syscom.c_error = 0;
}


/*
 *  Convert a channel number into a pointer
 *  to the corresponding channel structure.
 *  Return NULL if the channel number is
 *  bad (out of range).
 */
struct channel *
chcheck (chn)
unsigned int chn;
{
	if (chn >= CHNMAX)
		return (NULL);
	return (&channel[chn]);
}


/*
 *  close an RT11 channel
 */
rtclose (chn)
unsigned int chn;
{
	register struct channel *cp;
	int fd;

	if (chn >= CHNMAX)
		return;
	cp = &channel[chn];
	fd = cp->ch_fd;
	if (fd >= 0)
		close (fd);
#ifdef VAX
#ifdef VIRTUAL
	rtvclose (fd);
#endif
#endif
	cp->ch_fd = -1;
}


/*
 *  Close all RT11 I/O channels up to "max".
 */
closall (max)
{
	register unsigned int chan;

	for (chan = 0;  chan <= max;  ++chan)
		rtclose (chan);
}


/*
 *  Lookup a file given the file spec block.
 */
lookup (chn, dblk)
unsigned chn;
rad50 *dblk;
{
	char fname[50];

	makfnam (fname, dblk);
		/* printf("LOOKUP file '%s' on chanel %d\n", fname, chn); */
	syscom.c_error = flookup (chn, fname, RDWR);
}


#define	MAXNAM	(sizeof libpref + 15)
/*
 *  Lookup a file and return an error code.
 *  The error codes are:
 *	-1 - no error,
 *	 0 - channel already in use,
 *	 1 - cannot access file.
 *  If the file cannot be opened in the current directory,
 *  an attempt will be made to open it (always read-only) in
 *  the library directory.
 */
flookup (chn, name, mode)
unsigned int chn;
char *name;
{
	register struct channel *cp;
	char libfil[MAXNAM];
	char *strcop();

	if (chn >= CHNMAX  ||  (cp = &channel[chn])->ch_fd >= 0)
		return (0);
	cp->ch_err = NO;
	if ((cp->ch_fd = ttyfil(name)) >= 0)
		return (-1);
	strcop (name, strcop (libpref, libfil, libfil+MAXNAM), libfil+MAXNAM);
	if ((mode!=RDWR  ||  (cp->ch_fd=open(name,2)) < 0)   &&
	    (cp->ch_fd = open(name, 0)) < 0   &&
	    (cp->ch_fd = open(libfil, 0)) < 0)
		return (1);
#ifdef VAX
#ifdef VIRTUAL
	if (mode == VREAD)
		rtvopen (cp->ch_fd);
#endif
#endif
	putname (&cp->ch_ssb, name);
	return (-1);
}


/*
 *  Enter a file given the file spec block.
 */
enter (chn, dblk)
unsigned chn;
rad50 *dblk;
{
	char fname[50];

	makfnam (fname, dblk);
		/* printf("ENTER file '%s' on channel %d\n", fname, chn); */
	syscom.c_error = fenter (chn, fname);
}


/*
 *  Enter a file and return an error code.
 *  The error codes are:
 *	-1 - no error,
 *	 0 - channel already in use,
 *	 1 - cannot create file.
 */
fenter (chn, name)
unsigned int chn;
char *name;
{
	register int fd;
	register struct channel *cp;

	if (chn >= CHNMAX  ||  (cp = &channel[chn])->ch_fd >= 0)
		return (0);
	cp->ch_err = NO;
	if ((cp->ch_fd = ttyfil(name))  >=  0)
		return (-1);
	if ((fd = creat(name, 0644))  <  0)
		return (1);
	close (fd);
	if ((cp->ch_fd = open(name, 2))  <  0)
		return (1);
	putname (&cp->ch_ssb, name);
	return (-1);
}


/*
 *  If the argument file name refers to the terminal
 *  or the keyboard, return a duplicate of the appropriate
 *  standard I/O file descriptor.
 */
ttyfil (name)
char *name;
{
	register char *nm;

	nm = name;
	if (strpref (nm,"kb:"))
		return (dup(0));
	if (strpref(nm,"tt:") || strpref(nm,"tty:"))
		return (dup(1));
	return (-1);
}


/*
 */
rtread (chan, buff, wcnt, crtn, blk, r0)
unsigned int chan, wcnt, blk;
char *buff;
wordp crtn;
{
	register unsigned int c, xcnt;
	register struct channel *cp;

	c = chan;
	if (c >= CHNMAX  ||  (cp = &channel[c])->ch_fd < 0) {
		syscom.c_error = 2;
		return (r0);
	}
#ifdef VAX
#ifdef VIRTUAL
	xcnt = rtvread (cp->ch_fd, buff, wcnt, blk);
	if (xcnt == (unsigned int) ERROR) 
#endif
#endif
	{
		lseek (cp->ch_fd, ((long) blk) << 9, 0);
		xcnt = read (cp->ch_fd, buff, (int) (wcnt*2));
	}
	if (xcnt != (unsigned int) ERROR) {
		clear (buff + xcnt, buff + wcnt*2);
		xcnt = (xcnt + 1) / 2;
	}
	return (rtiofin (c, cp, wcnt, crtn, xcnt, r0));
}


/*
 */
rtwrite (chan, buff, wcnt, crtn, blk, r0)
unsigned int chan, wcnt, blk;
char *buff;
wordp crtn;
{
	register unsigned int c, xcnt;
	register struct channel *cp;

	c = chan;
	if (c >= CHNMAX  ||  (cp = &channel[c])->ch_fd < 0) {
		syscom.c_error = 2;
		return (r0);
	}
	lseek (cp->ch_fd, ((long) blk) << 9, 0);
	xcnt = write (cp->ch_fd, buff, (int) (wcnt*2));
	if (xcnt != (unsigned int) ERROR)
		xcnt = (xcnt + 1) / 2;
	if (xcnt < wcnt)
		syscom.c_error = 0;
	return (rtiofin (c, cp, wcnt, crtn, xcnt, r0));
}


/*
 */
rtiofin (chan, acp, wcnt, crtn, xcnt, r0)
unsigned int chan, wcnt, xcnt;
wordp crtn;
struct channel *acp;
{
	register unsigned int cnp, xt;
	register struct channel *cp;

	cnp = crtn;
	xt = xcnt;
	cp = acp;
	cp->ch_err = NO;
	if (xt == (unsigned int) ERROR)
		cp->ch_err = YES;
	if (xt == 0  &&  xt != wcnt)
		syscom.c_error = 0;
	if (cnp == 1)
		return (r0);
	if (cnp == 0) {
		if (xt == (unsigned int) ERROR) {
			syscom.c_error = 1;
			xt = 0;
		}
		return (xt);
	}
	comprtn = cnp;
	compcsw = 0;
	if (cp->ch_err)
		compcsw = -1;
	compchn = chan;
	return (r0);
}


/*
 *  "Wait" for completion of I/O
 *  and set I/O error code.
 */
rtwait (chan)
unsigned int chan;
{
	register struct channel *cp;

	if (chan >= CHNMAX  ||  (cp = &channel[chan])->ch_fd < 0) {
		syscom.c_error = 0;
		return;
	}
	if (cp->ch_err)
		syscom.c_error = 1;
}


/*
 *  Delete a file.
 *  Error code 1 (file not found) is used
 *  for all errors.
 */
delete (dblk)
rad50 *dblk;
{
	char fname[50];

	makfnam (fname, dblk);
	if (unlink (fname)  <  0)
		syscom.c_error = 1;
}


/*
 *  Rename a file.
 *  Error code 1 (file not found) is used
 *  for all errors.
 *  NOTE that an existing file with the new
 *  name will be unlinked.
 */
 rename (dblk)
 rad50 *dblk;
 {
	char fname1[50], fname2[50];

	makfnam (fname1, dblk);
	makfnam (fname2, dblk+3);
	unlink(fname2);
	if (link(fname1,fname2) < 0  ||  unlink(fname1) < 0)
		syscom.c_error = 1;
}


/*
 *  Try to fit the argument file into the agument
 *  savestatus filename block.  If it is too big,
 *  put it someplace else and leave a pointer in
 *  the savestatus block.
 */
putname (assp, name)
struct ssblok *assp;
char *name;
{
	register struct ssblok *ssp;
	register char *np, *sp;
	char *strcop();

	ssp = assp;
	sp = ssp->ssn;
	np = name;
	do {
		*sp = EOS;
		if (*np != EOS)
			*sp = *np++;
	} while (++sp < ssp->ssn + SVSIZE);
	if (*np == EOS  &&  *name != EOS)
		return;
	((word*) ssp)[0] = 0;
	if (name >= fnstore  &&  name < fnstore+NAMSTO) {
		((word*) ssp)[1]  =  name - fnstore;
		return;
	}
	sp = fnbreak;
	((word*) ssp)[1]  =  sp - fnstore;
	sp = strcop (name, sp, fnstore + NAMSTO - 1) + 1;
	if (sp  >=  fnstore + NAMSTO) {
		printf ("\nfile name storage overflow!!\n");
		exit (1);
	}
	fnbreak = sp;
}


/*
 *  Return a pointer to the name of the file stored in
 *  the argument savestatus block.
 */
char *
getname (assp)
struct ssblok *assp;
{
	register struct ssblok *ssp;
	register char *np, *sp;
	static char name[SVSIZE+1];

	ssp = assp;
	if (((word*)ssp)[0]  ==  0)
		return (fnstore + ((word*)ssp)[1]);
	np = name;
	sp = ssp->ssn;
	do
		*np++ = *sp++;
	while (np  <  name + SVSIZE);
	*np = EOS;
	return (name);
}


/*
 *  Save I/O channel status in argument channel block.
 *  The I/O channel status really ought to be something
 *  other than the name of the open file, but this is
 *  really the best we can do under the circumstances.
 */
savesta (chn, cblkp)
unsigned int chn;
word *cblkp;
{
	register struct channel *cp;

	cp = chcheck (chn);
	if (cp == NULL  ||  cp->ch_fd < 0) {
		syscom.c_error = 1;
		return;
	}
	copy (cp->ch_ssb.ssn, (char*) cblkp, (char*) cblkp + SVSIZE);
	close (cp->ch_fd);
	cp->ch_fd = -1;
}


/*
 *  Reopen a saved channel.
 */
reopen (chn, cblkp)
unsigned int chn;
word *cblkp;
{
	syscom.c_error = flookup (chn, getname((struct ssblok*)cblkp), RDWR);
}


/*
 *  Return YES if the first string
 *  is the same as the second.
 */
streq (a, b)
register char *a, *b;
{
	do if (*a++ != *b)
		return (NO);
	while (*b++ != '\0');
	return (YES);
}


/*
 *  Return YES if the first string
 *  begins with the second.
 */
strpref (a, b)
register char *a, *b;
{
	do if (*b == '\0')
		return (YES);
	while (*a++ == *b++);
	return (NO);
}


char *
strcop (from, to, limit)
char *from, *to, *limit;
{
	register char *f, *t;

	f = from;
	t = to;
	while ((*t = *f++) != EOS)
		if (++t >= limit)
			--t;
	return (t);
}


/*****************************************************/


/*
 *  VIRTUAL input stuff
 */

/*
 *  Storage for the "virtual file" is allocated with sbrk().
 *  Therefore these routines may not be compatible with
 *  malloc(), free(), and things that use malloc() and free()
 *  (e.g. the standard I/O library).
 */

#ifdef	VAX
#ifdef	VIRTUAL

#define	MAXBLK	1024
#define	RTBSIZ	512
#define	RTBSZW	(RTBSIZ/2)

int	virtfd = -1;
int	blkbrk;
char	*vbase;
char	filmap[MAXBLK/8];


rtvopen (fd)
{
	register int n;
	char *sbrk();

	if (virtfd >= 0)
		return (ERROR);
	blkbrk = (lseek (fd, 0L, 2) + RTBSIZ-1) / RTBSIZ;
	if (blkbrk > MAXBLK)
		return (ERROR);
	vbase = sbrk (blkbrk * RTBSIZ);
	if (vbase == 0)
		return (ERROR);
	for (n = 0;  n < MAXBLK/8;  ++n)
		filmap[n] = 0;
	virtfd = fd;
	return (fd);
}


rtvclose (fd)
{
	if (fd == virtfd)
		virtfd = -1;
}


rtvread (fd, buff, wcnt, blk)
{
	register int n;
	int blkcnt, endblk;
	long offset;

	if (fd != virtfd)
		return (ERROR);
	offset = blk * RTBSIZ;
	blkcnt = (wcnt+(RTBSZW-1)) / RTBSZW;
	endblk = blk + blkcnt;
	if (endblk > blkbrk)
		return (ERROR);
	for (n = blk;  n < endblk;  ++n)
		if ((filmap[n/8] & 1<<(n%8))  ==  0)
			break;
	if (n < endblk) {
		n = blkcnt * RTBSIZ;
		lseek (fd, offset, 0);
		if (read (fd, vbase+offset, n)  !=  n)
			return (ERROR);
		for (n = blk;  n < endblk;  ++n)
			filmap[n/8] |= 1 << (n%8);
	}
	n = wcnt * 2;
	fastcopy (buff, vbase+offset, n);
	return (n);
}


/*
 */
fastcopy (to, from, bct)
char *to, *from;
unsigned int bct;
{
	asm("  movc3  12(ap), *8(ap), *4(ap)");
}

#endif
#endif
