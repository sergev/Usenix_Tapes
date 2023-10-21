/*
 * Get user name and home directory.  This version always goes to the
 * password file, so it cannot be subverted by modifying the environment.
 */

#include <stdio.h>
#include <pwd.h>

char *username, *userhome;

pgetuser()
{
	register struct passwd *p;
	char *savestr();
	struct passwd *getpwuid();

	if ((p = getpwuid(getuid())) == NULL)
		xerror("Cannot get user's name");
	username = savestr(p->pw_name);
	userhome = savestr(p->pw_dir);
}
