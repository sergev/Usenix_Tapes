#
#include "bbuf.h"
#include <error.h>
#include <site.h>
#define	nextarg		{++argv; --argc;}		/* function to move to next argument */

char *prog_id "~|^' formdict  Release 3 Version 0\n";
struct bufr dictbuf;

char hname[512] DICTHASH;

main(argc, argv)
int	argc;							/* count of command line arguments */
char	**argv;							/* vector to pointer list of command line arguments */
{
        long htbl[26*26];
        register long *lp;
        register long *lp2;
        long temp;
        int fd, idx;
        register int chr;
        int schr;
        char line[128];
        char *linep;
        long getlocb();


	/* COMMAND LINE ARGUMENT GATHERING SECTION */
	nextarg;			/* skip over the arg with command name (arg 0)*/
	while (argc > 0 && **argv == '-')		/* while there are arguments, and */
							/* they start with a dash */
	{
		for(;;) {				/* until end of string is detected */
			switch (*++*argv)		/* interpret each flag letter */
			{
			case '\0':			/* end of string, go on to next argument */
				goto ugh_a_goto;
			default:			/* No flags allowed */
				exit(EFLAG);
			}
		}
		ugh_a_goto:
		nextarg;
	}

	/* MAIN BODY OF PROGRAM */

        lp = &htbl[-1];
        if(argc!=2)
        {
		exit(EFLAG);
        }
        if (openb (argv[0], &dictbuf, 0) < 0) {
                printf ("No dictionary\n");
		exit(ENOENT);
        }
        fd = creat (argv[1], 0644);
        if (fd < 0) {
                printf ("Can't open %s\n", DICTHASH);
                exit(EACCES);
        }
        chr = 0;
        for (;;) {
                do {
                        temp = getlocb (&dictbuf);
                        if (word (line)) goto done;
                        linep = line;
                        schr = hchr(&linep); /* start char of line */
                        if (schr != chr) {
                                while (++lp < &htbl[26*26]) *lp = -1;
                                write (fd, htbl, sizeof htbl);
                                lp = &htbl[-1];
#       ifdef debug
        printf ("wordin dict=%s\n",line);
#       endif
                                chr = schr;
                        }
                        idx = hash (linep);
                        lp2 = &htbl[idx];
                } while (lp2 == lp);    /* scan for different hash entry */
                if (lp > lp2) {
                        printf ("word out of order: %s\n", line);
                        continue;
                }
                while (++lp < lp2) {
                        *lp = -1;
                }
                *lp = temp;
#       ifdef debug
        printf ("new entry %s %c%c%c\n",
                line,
                schr+'a',
                idx/26+'a',
                idx%26+'a');
#       endif
        }

done:
        while (++lp < &htbl[26*26]) *lp = -1;
        write (fd, htbl, sizeof htbl);
        close (fd);
}

int word (p)
char *p;
{
        int c;
        while ((c = getbc(&dictbuf)) >= 0 && c != '\n') {
                *p++ = c;
        }
        *p++ = '\0';
        if (c < 0) return c;
        return 0;
}
