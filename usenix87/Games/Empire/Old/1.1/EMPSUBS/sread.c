/*
        There are two versions of sread.c in here.  One for 4.2 BSD, and
        one for other (SysV) systems.
*/
#include        "empdef.h"

#define BSD /* Jon */	/* HACKed to get the right version for Xenix 1.3.5 */

#ifdef BSD
#include        <sgtty.h>

sread(buf, maxl)
char    *buf;
int     maxl;
{
        register        int     i;
        struct  sgttyb  ttyb;
        unsigned short  flags;

        ioctl(0, TIOCGETP, &ttyb);
        flags = ttyb.sg_flags;
        ttyb.sg_flags &= ~ECHO;
        ioctl(0, TIOCSETN, &ttyb);
        i = read(0, buf, maxl);
        ttyb.sg_flags = flags;
        ioctl(0, TIOCSETN, &ttyb);
        return(i);
}
#else
#include        <termio.h>

sread(buf, maxl)
char    *buf;
int     maxl;
{
        register        int     i;
        struct  termio  ttyb;
        unsigned short  flags;

        ioctl(0, TCGETA, &ttyb);
        flags = ttyb.c_lflag;
        ttyb.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
        ioctl(0, TCSETAF, &ttyb);
        i = read(0, buf, maxl);
        ttyb.c_lflag = flags;
        ioctl(0, TCSETAF, &ttyb);
        return(i);
}
#endif  BSD
#undef BSD

