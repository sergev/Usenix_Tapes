/*
**  Return current working directory.  Something for everyone.
*/
/* LINTLIBRARY */
#include "shar.h"
RCS("$Header: cwd.c,v 1.3 87/03/02 11:03:21 rs Exp $")


#ifdef	PWDGETENV
/* ARGSUSED */
char *
Cwd(p, i)
    char	*p;
    int		 i;
{
    char	*q;

    return((q = getenv(PWDGETENV)) ? strcpy(p, q) : NULL);
}
#endif	/* PWDGETENV */


#ifdef	GETWD
/* ARGSUSED1 */
char *
Cwd(p, size)
    char	*p;
    int		 size;
{
    return(getwd(p) ? p : NULL);
}
#endif	/* GETWD */


#ifdef	GETCWD
char *
Cwd(p, size)
    char	*p;
    int		 size;
{
    return(getcwd(p, size) ? p : NULL);
}
#endif	/* GETCWD */


#ifdef	PWDPOPEN
extern FILE	*popen();
char *
Cwd(p, size)
    char	*p;
    int		 size;
{
    FILE	*F;
    int		 i;

    if (F = popen("exec pwd", "r")) {
	if (fgets(p, size, F) && p[i = strlen(p) - 1] == '\n') {
	    p[i] = '\0';
	    (void)fclose(F);
	    return(p);
	}
	(void)fclose(F);
    }
    return(NULL);
}
#endif	/* PWDPOPEN */
