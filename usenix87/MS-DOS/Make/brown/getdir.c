#include "lar.h"
getdir (f)
fildesc f;
{
    extern int nslots;
    extern struct ludir ldir[MAXFILES];
    char *getname();

    lseek(f, 0L, 0);		/* rewind f */

    if (_read(f, (char *) &ldir[0], DSIZE) != DSIZE)
	error ("No directory\n");

    if ( !equal(getname(ldir[0].l_name, ldir[0].l_ext), "larformt.arc") ) {
        error("This is not a LAR format archive!!");
    }

    if (lwtol(ldir[0].l_datetime) != -1L)
        error("This is not a LAR format archive!!");

    nslots = (int) (lwtol (ldir[0].l_len)/DSIZE);

    if (_read (f, (char *) & ldir[1], DSIZE * nslots) != DSIZE * nslots)
	error ("Can't read directory - is it a library?");
}

/* convert nm.ex to a Unix style string */
char   *getname (nm, ex)
char   *nm, *ex;
{
    static char namebuf[14];
    register char  *cp, *dp, *ocp;

    for (cp = namebuf, dp = nm; *dp != ' ' && dp != nm+8;)
	*cp++ = isupper (*dp) ? tolower (*dp++) : *dp++;
    *cp++ = '.';
    ocp = cp;

    for (dp = ex; *dp != ' ' && dp != ex+3;)
	*cp++ = isupper (*dp) ? tolower (*dp++) : *dp++;

    if (cp == ocp)		/* no extension */
        --cp;			/* eliminate dot */

    *cp = '\0';
    return namebuf;
}

cant (name)
char   *name;
{
extern int  errno;
extern char *sys_errlist[];

fputs(name, stderr);
fputs(": ", stderr);
fputs(sys_errlist[errno], stderr);
fputs("\n", stderr);
fflush(stderr);
exit (1);
}

error (str)
char   *str;
{
fputs("lar: ", stderr);
fputs(str, stderr);
fputs("\n", stderr);
fflush(stderr);
exit (1);
}


/*
 * This itoa doesn't call in the floating point library:
 *   CI C86 does an sprintf!
 */
itoa(val, buf)
int val;
char *buf;
{
	register int i;
	int j;
	register char tbuf[10];

	if (val == 0) {
		buf[0] = '0';
		buf[1] = '\0';
		return;
	}

	i = 9;

	while (val != 0) {
		tbuf[i--] = (val % 10) + '0';
		val /= 10;
	}

	i++;

	for (j = 0; i <= 9; i++, j++)
		buf[j] = tbuf[i];

	buf[j] = '\0';
}
