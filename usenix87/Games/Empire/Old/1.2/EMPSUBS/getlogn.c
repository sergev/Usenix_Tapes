/*
** Compile: cc -O -c -q getlogn.c; ar r empsub.a getlogn.o
*/

char *
getlogn(uid)                                      /* return logname of uid */
{
        register char *cp;
        static char buf[128] = { '\0' };
        static int lastuid = 0;

        if (*buf != '\0' && lastuid == uid && uid != 0)
            return(buf);
        if (getpw( uid, buf) != 0)
            return("????");
        for (cp = buf; *cp && *cp != ':'; cp++);
        *cp = '\0';
        return(buf);
}
