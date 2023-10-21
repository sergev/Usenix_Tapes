#
/*
 * data.h - initialize various system variables at link time
 * Since multiple copies of initialized data upset the loader,
 * this file can only be #included but once, in main.c.
 * --ghg 01/22/78.
 */

int	maxblk	MAXBLK;		/* max size of non SU file */
int	maxblk2	MAXBLK2;	/* max size for file (uid < MAXBLKU) */
# ifdef XBUF
char	*bufsiz	NBUF2*8;	/* really NBUF2*512/64 but that overflows */
char	*clsiz	{(NCLIST*8+63)/64}; /* clist size in 32 word units */
char	*prsiz	{(sizeof proc+63)/64}; /* proc size in 32 word units */
# endif
# ifdef MX
char	*ntsiz	128;	/* for now */
# endif
