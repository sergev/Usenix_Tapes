#ifndef lint
static char	sccsid[] = "@(#)gethostnam.c	6.1 (down!honey) 85/01/21";
static char	rcsid[] = "$Header: gethostnam.c,v 6.5 86/10/07 15:14:09 sob Exp $";
#endif lint

#ifndef GETHOSTNAME
#include "uuconf.h"
#ifdef DOUNAME
#include <sys/utsname.h>
#endif

void
gethostname(name, len)
char	*name;
{
	FILE	*whoami, *fopen(), *popen();
	char	*ptr, *index();
#if defined(DOUNAME) && !defined(SYSTEMNAME)
	struct utsname utsn;
#endif

	*name = '\0';
#ifndef SYSTEMNAME
#ifdef DOUNAME
	if (uname(&utsn) != -1)
	{
		strcpy(name,utsn.nodename);
		len = strlen(name);
	  	return;
	}
#endif
#endif

#ifdef SYSTEMNAME
	/* try /usr/lib/uucp/SYSTEMNAME */
	if ((whoami = fopen("/usr/lib/uucp/SYSTEMNAME", "r")) != 0) {
		(void) fgets(name, len, whoami);
		(void) fclose(whoami);
		if ((ptr = index(name, '\n')) != 0)
			*ptr = '\0';
	}
	if (*name)
		return;
#endif
	/* try /usr/include/whoami.h */
	if ((whoami = fopen("/usr/include/whoami.h", "r")) != 0) {
		while (!feof(whoami)) {
			char	buf[100];

			if (fgets(buf, 100, whoami) == 0)
				break;
			if (sscanf(buf, "#define sysname \"%[^\"]\"", name))
				break;
		}
		(void) fclose(whoami);
		if (*name)
			return;
	}

	/* ask uucp */
	if ((whoami = popen("uuname -l", "r")) != 0) {
		(void) fgets(name, len, whoami);
		(void) pclose(whoami);
		if ((ptr = index(name, '\n')) != 0)
			*ptr = '\0';
	}
	if (*name)
		return;
	
	/* failure */
	return;
}
#endif GETHOSTNAME

