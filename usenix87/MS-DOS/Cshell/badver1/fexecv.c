#ifdef DEBUG
#ifndef FILE
#include <stdio.h>
#endif
#include <ctype.h>
#include <debug.h>
#endif


/* Copyright (C) 1983, 1984 by Manx Software Systems */
/* modified by kent williams to employ environment managed in env.c */
extern int env_paragraph;

fexecv(path, argv)
char *path, **argv;
{
	register char *cp, *xp;
	int i;
	char buffer[258];
	char fcb1[16], fcb2[16];

	cp = buffer+1;
	i = 1;
	if (*argv) {
		++argv;			/* skip arg0, used for unix (tm) compatibility */
		while (xp = *argv++) {
			if (i == 1)
				fcbinit(xp, fcb1);
			else if (i == 2)
				fcbinit(xp, fcb2);
			while (*xp) {
				if (cp >= buffer+256)
					goto done;
				*cp++ = *xp++;
			}
			*cp++ = ' ';
			++i;
		}
	}
done:
	buffer[0] = cp - (buffer+2);
	/* terminate string */
	buffer[buffer[0]+1] = 0;
#ifdef DEBUG
		fprintf(stderr,"\nbuffer[0] = %d\n",buffer[0]);
		for (i = 1; buffer[i] ; i++)
		{
			if (isprint(buffer[i]))
				putchar(buffer[i]);
			else
				fprintf("buffer[%d] = %d\n",i,buffer[i]);
		}
		crlf();
#endif
	return fexec(path, env_paragraph, buffer, fcb1, fcb2);
}

