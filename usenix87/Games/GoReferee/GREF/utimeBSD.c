#ifdef	NOUTIME
utiminit()	{ return(0); }
utime()		{ return(0); }
kidutime()	{ return(0); }
int	ycptp;	/* set to proc.p_cptr (youngest child's proc table pointer) */
#else
#include	<sys/param.h>
#include	<sys/dir.h>
#include	<sys/user.h>
#include	<sys/proc.h>
#include	<sys/vm.h>
#include	<machine/pte.h>
#include	<nlist.h>
#include	<stdio.h>
extern	char	fmtbuf[];
/*
**	UTIMINIT() -- Do one-time init stuff for ...
**	UTIME(pid) -- Return the user time, in seconds, for process pid
**		Also set global ycptp.
**	KIDUTIME() -- Return the current total of terminated child process
**		user times (seconds, ignoring microseconds).
**	psl 5/84
*/

#define	UNIX	"/vmunix"
#define	KMEM	"/dev/kmem"
#define	MEM	"/dev/mem"
#define	SWAP	"/dev/drum"

static	struct	nlist	nl[]	= {
#define	vproc		(nl[0].n_value)
	{ "_proc" },
#define	vnproc		(nl[1].n_value)
	{ "_nproc" },
#define	vUsrptmap	(nl[2].n_value)
	{ "_Usrptmap" },
#define	vusrpt		(nl[3].n_value)
	{ "_usrpt" },
	0
};

static	int	kfh, mfh, sfh, nproc;
static	struct	pte *Usrptma, *usrpt;
static	struct	proc *procp;
union	{
	struct	user	page;			/* where the data really goes */
	char		pad[NBPG][UPAGES];	/* to pad out to size read */
} uzer;
#define	usr	uzer.page		/* not as bad a hack as many to come */

int	ycptp;	/* set to proc.p_cptr (youngest child's proc table pointer) */

utiminit()
{
	nlist(UNIX, nl);
	if ((kfh = vopen(KMEM)) < 0
	 || (mfh = vopen(MEM)) < 0
	 || (sfh = open(SWAP)) < 0)
	    return(-1);
	procp = (struct proc *) kmemint(vproc);
	nproc = kmemint(vnproc);
	Usrptma = (struct pte *) vUsrptmap;
	usrpt = (struct pte *) vusrpt;
	return(0);
}

static
vopen(file)
char	*file;
{
	register int i;

	if ((i = open(file, 0)) < 0)
	    perror(file);
	return(i);
}

static
kmemint(addr)
unsigned long	addr;
{
	long data;

	lseek(kfh, addr, 0);
	if (read(kfh, &data, sizeof data) != sizeof data)
	    perror(KMEM);
	return(data);
}

int
utime(pid)
{
	register int i, s, n;
	struct proc p;
	extern int errno;

	for (i = 0; i < nproc; i++) {
	    s = (int) &procp[i];
	    n = lseek(kfh, (long) s, 0);
	    if (n != s) {
		sprintf(fmtbuf, "lseek(kmem, %x, 0) returns %d", s, n);
		log(fmtbuf);
	    }
	    s = sizeof p;
	    n = read(kfh, &p, s);
	    if (n != s) {
		sprintf(fmtbuf, "read(kmem, proc, %d) returns %d", s, n);
		log(fmtbuf);
		sprintf(fmtbuf, "errno=%d", errno);
		log(fmtbuf);
		return(0);
	    }
	    if (p.p_pid == pid)
		break;
	}
	if (i >= nproc)		/* did we find the proc table entry? */
	    return(0);
	ycptp = (int) p.p_cptr;
	if (readusr(&p) == 0)
	    return(0);
	return(usr.u_ru.ru_utime.tv_sec);
}

static
readusr(pp)
struct	proc	*pp;
{
	register int i, n, s, ncl;
	char *cp;
	struct pte *pteaddr, apte;
	struct pte arguutl[UPAGES+CLSIZE];
	extern int errno;

	if ((pp->p_flag & SLOAD) == 0) {
	    lseek(sfh, (long)dtob(pp->p_swaddr), 0);
	    s = sizeof usr;
	    n = read(sfh, &usr, s);
	    if (n == s)
		return (1);
	    sprintf(fmtbuf, "read(swap, upage, %d) returns %d", s, n);
oops:
	    log(fmtbuf);
	    sprintf(fmtbuf, "errno=%d", errno);
	    log(fmtbuf);
	    return (0);
	}
	/* indirect pte from kmem */
	pteaddr = &Usrptma[btokmx(pp->p_p0br) + pp->p_szpt - 1];
	lseek(kfh, (long)pteaddr, 0);
	s = sizeof apte;
	n = read(kfh, (char *)&apte, s);
	if (n != s) {
	    sprintf(fmtbuf, "read(kmem, apte, %d) returns %d", s, n);
	    goto oops;
	}
	s = (long)ctob(apte.pg_pfnum+1) - (UPAGES+CLSIZE) * sizeof (struct pte);
	lseek(mfh, (long) s, 0);
	s = sizeof arguutl;
	n = read(mfh, (char *)arguutl, s);
	if (n != s) {
	    sprintf(fmtbuf, "read(mem, arguutl, %d) returns %d", s, n);
	    goto oops;
	}
	ncl = (sizeof usr + NBPG*CLSIZE - 1) / (NBPG*CLSIZE);
	cp = (char *) &usr;
	for (i = 0; i < ncl * CLSIZE; i += CLSIZE) {
	    s = ctob(arguutl[CLSIZE+i].pg_pfnum);
	    lseek(mfh, (long) s, 0);
	    s = CLSIZE * NBPG;
	    n = read(mfh, cp, s);
	    if (n != s) {
		sprintf(fmtbuf, "read(mem, upage, %d) returns %d", s, n);
		goto oops;
	    }
	    cp += s;
	}
	return (1);
}

kidutime()
{
	struct rusage ugh;

	getrusage(RUSAGE_CHILDREN, &ugh);
	return(ugh.ru_utime.tv_sec);
}

#endif	NOUTIME
