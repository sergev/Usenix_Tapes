#ifndef lint
static char *what = "@(#)one.c 11/29/85 Warren Lavallee";
#endif

#include <pwd.h>
#include <stdio.h>

struct passwd  *pw;
struct passwd  *getpwnam ();

one (name)
char   *name;
{
    (void) setpwent ();
    if (pw = getpwnam (name)) {
	(void) printf ("%s      \tuid: %3d\tgid: %3d\n",
		pw -> pw_name,
		pw -> pw_uid, pw -> pw_gid);
	return (1);
    }
    (void) endpwent ();
    return (0);
}
