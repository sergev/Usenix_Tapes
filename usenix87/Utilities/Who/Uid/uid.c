#ifndef lint
static char *what = "@(#)uid.c 11/29/85 Warren Lavallee";
#endif

#include <pwd.h>
#include <stdio.h>

struct passwd  *pw;
struct passwd  *getpwuid ();

uid (uid)	/* find a uid in /etc/passwd and prints it */
int     uid;
{
    (void) setpwent ();
    if (pw = getpwuid (uid)) {
	(void) printf ("%s      \tuid: %3d\tgid: %3d\n",
		pw -> pw_name,
		pw -> pw_uid, pw -> pw_gid);
	return (1);
    }
    (void) endpwent ();
    return (0);
}
