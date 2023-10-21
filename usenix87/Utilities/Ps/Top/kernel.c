/*
 *  Top - a top users display for Berkeley Unix
 *  
 *  This file contains all the routines that retrieve values from
 *  kernel and user memory.
 */

#include <stdio.h>
#if defined(FOUR_ONE) || defined(pyr)
#include <sys/pte.h>
#else
#include <machine/pte.h>
#endif
#include <sys/param.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/proc.h>

#include "top.local.h"

/* useful externals */
extern int errno;
extern char *sys_errlist[];

static int kmem = -1;
static int mem = -1;

init_kernel()
{
    /* open kmem and mem */
    if ((kmem = open(KMEM, 0)) < 0)
    {
	perror(KMEM);
	exit(20);
    }
    if ((mem = open(MEM, 0)) < 0)
    {
	perror(MEM);
	exit(21);
    }

}

/*
 *  getu(p, u) - get the user structure for the process whose proc structure
 *	is pointed to by p.  The user structure is put in the buffer pointed
 *	to by u.  Return 0 if successful, -1 on failure (such as the process
 *	being swapped out).
 */

getu(p, u)

register struct proc *p;
struct user *u;

{
    struct pte uptes[UPAGES];
    register caddr_t upage;
    register struct pte *pte;
    register nbytes, n;

    /*
     *  Check if the process is currently loaded or swapped out.  The way we
     *  get the u area is totally different for the two cases.  For this
     *  application, we just don't bother if the process is swapped out.
     */
    if ((p->p_flag & SLOAD) == 0)
    {
	return(-1);
    }

    /*
     *  Process is currently in memory, we hope!
     */
    if (!getkval(p->p_addr, uptes, sizeof(uptes), "!p->p_addr"))
    {
	/* we can't seem to get to it, so pretend it's swapped out */
	return(-1);
    } 
    upage = (caddr_t)u;
    pte = uptes;
    for (nbytes = sizeof(struct user); nbytes > 0; nbytes -= NBPG)
    {
    	lseek(mem, pte++->pg_pfnum * NBPG, 0);
	n = MIN(nbytes, NBPG);
	if (read(mem, upage, n) != n)
	{
	    /* we can't seem to get to it, so pretend it's swapped out */
	    return(-1);
	}
	upage += n;
    }
    return(0);
}

/*
 *  getkval(offset, ptr, size, refstr) - get a value out of the kernel.
 *	"offset" is the byte offset into the kernel for the desired value,
 *  	"ptr" points to a buffer into which the value is retrieved,
 *  	"size" is the size of the buffer (and the object to retrieve),
 *  	"refstr" is a reference string used when printing error meessages,
 *	    if "refstr" starts with a '!', then a failure on read will not
 *  	    be fatal (this may seem like a silly way to do things, but I
 *  	    really didn't want the overhead of another argument).
 *  	
 */

getkval(offset, ptr, size, refstr)

long offset;
int *ptr;
int size;
char *refstr;

{
    if (lseek(kmem, offset, 0) == -1)
    {
	if (*refstr == '!')
	{
	    refstr++;
	}
	fprintf(stderr, "%s: lseek to %s: %s\n",
	    KMEM, refstr, sys_errlist[errno]);
	quit(22);
    }
    if (read(kmem, ptr, size) == -1)
    {
	if (*refstr == '!')
	{
	    /* we lost the race with the kernel, process isn't in memory */
	    return(0);
	} 
	else 
	{
	    fprintf(stderr, "%s: reading %s: %s\n",
		KMEM, refstr, sys_errlist[errno]);
	    quit(23);
	}
    }
    return(1);
}
