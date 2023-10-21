fexecvp(name, argv)
char *name, **argv;
{
	register char *cp, *xp;
	int result;
	char *getenv(), path[64];

	if (666 != (result = tryexec("", name, argv)))
		return result;
	if ((cp = getenv("PATH")) != 0) {
		while (*cp) {
			xp = path;
			while (*cp) {
				if (*cp == ';') {
					++cp;
					break;
				}
				*xp++ = *cp++;
			}
			*xp = 0;
			if (path[0] != 0)
				if (666 != (result = tryexec(path, name, argv)))
					return result;
		}
	}
	return 666;
}

static
tryexec(dir, name, argv)
char *dir, *name, **argv;
{
	char newname[64];
	register char *cp;
	char *rindex(),*index();

	strcpy(newname, dir);
	if (((cp = index(newname, '/')) || (cp = index(newname, '\\')))
				&& *(cp+1) != '\0')
		strcat(newname, "/");
	strcat(newname, name);
	if (index(name, '.') == 0) {
		strcat(newname, ".com");
		if (666 != fexecv(newname, argv))
			return wait();
		strcpy(rindex(newname,'.'), ".exe");
	}
	if (666 != fexecv(newname, argv))
			return wait();
	return 666;
}
