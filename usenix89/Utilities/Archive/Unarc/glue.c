/*
**  Subroutine to call the shell archive parser.  This is "glue"
**  between unshar and the parser proper.
*/
#include "shar.h"
RCS("$Header: glue.c,v 1.1 87/03/09 16:24:17 rs Exp $")


/*
**  Copy the input to a temporary file, then call the shell parser.
*/
BinSh(Name, Stream, Pushback)
    char		*Name;
    register FILE	*Stream;
    char		*Pushback;
{
    static char		 TEMP[] = "/tmp/shellXXXXXX";
    register FILE	*F;
    char		 buff[BUFSIZ];
    char		*vec[MAX_WORDS];

    Interactive = Name == NULL;
    File = mktemp(TEMP);
    F = fopen(File, "w");
    (void)fputs(Pushback, F);
    while (fgets(buff, sizeof buff, Stream))
	(void)fputs(buff, F);
    (void)fclose(Stream);

    if ((Input = fopen(TEMP, "r")) == NULL)
	fprintf(stderr, "Can't open %s, %s!?\n", TEMP, Ermsg(errno));
    else
	while (GetLine(TRUE))
	    if (Argify(vec))
		(void)Exec(vec);

    (void)unlink(TEMP);
}
