#
#define NBUF2 10
#define MAXBLK 1111
/*
 * data.h - initialize various system variables at link time
 * Since multiple copies of initialized data upset the loader,
 * this file can only be #included but once, in main.c.
 * --ghg 01/22/78.
 */

int	maxblk	MAXBLK;		/* max size of non SU file */
extern end();
char	*bufsiz		NBUF2*512/64;
main()
{
	exit();
}
