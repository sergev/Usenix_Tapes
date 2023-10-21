#define D_FILES
#include        "empdef.h"

char *
mailbox(cno)
int     cno;
{
        register char   *cp;
        char    *copy();
        static char     buf[64];

        cp = copy(telfil, buf);
        *cp++ = (cno/10) + '0';
        *cp++ = (cno%10) + '0';
        *cp = '\0';
        return(buf);
}
