#ifndef lint
#ifndef NSCCS
static char sccsid[] = "%W%";
#endif
#endif

char *
sname(nam)
register char *nam;
{
	register char *slash;

	slash = nam;
	while (*nam != '\0') {
		if (*nam++ == '/')
			slash = nam;
	}
	return(slash);
}
