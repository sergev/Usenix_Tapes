
#include <ctype.h>

#define	SDXLEN	4

char *
soundex(name)
char	*name;
{
	static char	buf[SDXLEN+1];
	register char	c, lc, prev = '0';
	register int	i;

	strcpy(buf, "a000");

	for (i = 0; *name && i < SDXLEN; name++)
		if (isalpha(*name)) {
			lc = tolower(*name);
			c = "01230120022455012623010202" [lc-'a'];
			if (i == 0 || (c != '0' && c != prev)) {
				buf[i] = i ? c : lc;
				i++;
			}
			prev = c;
		}

	return buf;
}

-----
And a little driver for it:
-----

main()
{
	char	line[64];

	while (gets(line))
		puts(soundex(line));
	return 0;
}
