/*
**  Return name of this host.  Something for everyone.
*/
/* LINTLIBRARY */
#include "shar.h"
RCS("$Header: host.c,v 1.5 87/03/02 11:03:26 rs Exp $")


#ifdef	HOST
char *
Host()
{
    return(HOST);
}
#endif	/* HOST */


#ifdef	GETHOSTNAME
char *
Host()
{
    static char		 buff[64];

    return(gethostname(buff, sizeof buff) < 0 ? "SITE" : buff);
}
#endif	/* GETHOSTNAME */


#ifdef	UNAME
#include <sys/utsname.h>
char *
Host()
{
    static struct utsname	 U;

    return(uname(&U) < 0 ? "SITE" : U.nodename);
}
#endif	/* UNAME */


#ifdef	UUNAME
extern FILE	*popen();
char *
Host()
{
    static char		 buff[50];
    char		*p;

    if (F = popen("exec uuname -l", "r")) {
	if (fgets(buff, sizeof buff, F) == buff && (p = IDX(name, '\n'))) {
	    (void)pclose(F);
	    *p = '\0';
	    return(buff);
	}
	(void)pclose(F);
    }
    return("SITE");
}
#endif	/* UUNAME */


#ifdef	WHOAMI
char *
Host()
{
    static char		 name[64];
    register FILE	*F;
    register char	*p;
    char		 buff[100];

    /* Try /etc/whoami; look for a single well-formed line. */
    if (F = fopen("/etc/whoami", "r")) {
	if (fgets(name, sizeof name, F) && (p = IDX(name, '\n')))
	    (void)fclose(F);
	    *p = '\0';
	    return(name);
	}
	(void)fclose(F);
    }

    /* Try /usr/include/whoami.h; look for #define sysname "foo" somewhere. */
    if (F = fopen("/usr/include/whoami.h", "r")) {
	while (fgets(buff, sizeof buff, F))
	    /* I don't like sscanf, nor do I trust it.  Sigh. */
	    if (sscanf(buff, "#define sysname \"%[^\"]\"", name) == 1) {
		(void)fclose(F);
		return(name);
	    }
	(void)fclose(F);
    }
    return("SITE");
}
#endif /* WHOAMI */
