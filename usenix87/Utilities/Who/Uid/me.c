#ifndef lint
static char *what = "@(#)me.c 11/29/85 Warren Lavallee";
#endif

#include <stdio.h>
#include <pwd.h>

struct passwd  *pw;
struct passwd  *getpwnam ();

int     myuid;

me () {
    myuid = getuid ();

    (void) setpwent ();
    if (pw = getpwuid (myuid)) {
	(void) printf ("%s      \tuid: %3d\tgid: %3d\n",
		pw -> pw_name,
		pw -> pw_uid, pw -> pw_gid);
	return (1);
    }
    else {
	(void) fprintf (stderr, "Sorry old chap, but you don't exist\!\n");
	exit (2);
    }
    (void) endpwent ();
    return (0);
}
