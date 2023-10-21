#ifndef lint
static char *what = "@(#)all.c 11/29/85 Warren Lavallee";
#endif

/*  all.c-- prints out all the entrys in the /etc/passwd file */

#include <pwd.h>
#include <stdio.h>
#include "more.h"		/* defines MORE */

struct passwd  *pw;
struct passwd  *getpwent ();

FILE * more, *popen ();

all () {			/* prints all entrys if file */
    if (!(more = popen (MORE, "w"))) {/* open pipe */
	(void) fprintf (stderr, "cannot activate %s\n", MORE);
	exit (2);
    }

    (void) setpwent ();
    while (pw = getpwent ())
	if (strlen (pw -> pw_passwd) >= 5){	/* see if active */
	    (void) fprintf (more, "%s      \tuid: %3d\tgid: %3d\n",
		    pw -> pw_name,
		    pw -> pw_uid, pw -> pw_gid);
	}
    (void) endpwent ();
    (void) pclose (more);
}
