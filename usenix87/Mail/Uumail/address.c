/*
 * address - run opath to see what a translated RFC822 address will come
 * out as.
 *
 * By E. Roskos 1/16/85
 * $Log:	address.c,v $
 * Revision 4.0  86/11/17  16:01:45  sob
 * Release version 4.0 -- uumail
 * 
 * Revision 3.3  86/10/01  15:48:13  sob
 * removed references to now-defunct CONFIGFILE
 * 
 * Revision 3.2  86/07/11  17:57:29  sob
 * Checkpoint in adaptation for resolve
 * 
 * Revision 3.1  86/06/10  16:47:56  sob
 * uswitch per complaint.
 * Stan
 * 
 * Revision 3.0  86/03/14  12:04:19  sob
 * Release of 3/15/86 --- 3rd Release
 * 
 * Revision 1.4  85/12/26  15:47:45  sob
 * Added modifications suggested by terry%owl@rand-unix.ARPA
 * 
 * Revision 1.3  85/11/24  14:50:01  sob
 * Added corrections provided by regina!mark
 * 
 * Revision 1.2  85/09/16  18:31:53  sob
 * Added DEBUG flag
 * 
 * Revision 1.1  85/09/16  17:50:24  sob
 * Initial revision
 * 
 */
#define _DEFINE
#include "uuconf.h"

static char rcsid[] = "$Header: address.c,v 4.0 86/11/17 16:01:45 sob Exp $";

EXTERN char *paths;
char *opath(), *oupath();
int Debug;

main(argc,argv)
int argc;
char **argv;
{
char *p;
char user[BUFSIZ];
char domain[BUFSIZ];
paths = DATABASE;
handle = ALL;
	if (argc < 2)
	{
		fprintf(stderr,"usage: %s rfcaddress [...]\n",
			argv[0]);
		exit(1);
	}

	while (--argc)
	{
		p = *++argv;
		if (*p=='-')
		{
			switch(*++p)
			{
			case 'd': Debug++;
			          continue;
			default:  printf("unknown switch: %c\n",*p);
				  continue;
			}
		}
	        resolve(p, user, domain);
		printf("%s: ",p);

        	if(domain[0] == '\0')
                	printf("%s\n", user);
	        else if(user[0] == '\0')
        	        printf("%s\n", domain);
	        else
        	        printf("%s!%s\n", user, domain);
	}

	exit(0);
}
