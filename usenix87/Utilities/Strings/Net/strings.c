/*
 * Public domain strings
 *
 * Conforms (closely) to the Berkeley 4.2 BSD manual page.
 *
 * Lee Ward
 * University of New Mexico
 * 4/14/84
 */

# include <stdio.h>
# include <a.out.h>
# include <ctype.h>

/* Is the below right? Oh well.... */
# ifdef V7
# define	N_BADMAG(x)	!((x).a_magic == A_MAGIC1 || \
				    (x).a_magic == A_MAGIC2 || \
				    (x).a_magic == A_MAGIC3 || \
				    (x).a_magic == A_MAGIC4)
# define	N_TXTOFF(x)	(x).a_magic == A_MAGIC2 || \
				    (x).a_magic == A_MAGIC3 ? \
				    8 * 1024 : sizeof(x)
# endif

int	countflag = 0;
int	dataflag = 0;

char	*Usgstr = "[-] [-o] [-minlength] file...";
int	minlength = 4;

extern int errno;

main (argc, argv)
	int argc;
	char *argv[];
{
	int x;
	char *aptr;
	struct exec exhdr;

	argv[argc] = NULL;
	for (x = 1; argc > 1 && *(aptr = argv[x]) == '-'; x++,argc--) {
		++aptr;
		if (*aptr == NULL)
			dataflag++;
		else if (*aptr == 'o')
			countflag++;
		else if (isdigit(*aptr))
			minlength = atoi(aptr);
		else {
			fprintf (stderr, "Usage: %s %s\n", argv[0], Usgstr);
			exit (1);
		}
	}

	do {
		if (argc > 1)
			if (freopen(argv[x++], "r", stdin) == NULL) {
				perror (argv[x - 1]);
				exit (errno);
			}
		if (!dataflag &&
		    fread(&exhdr, sizeof(exhdr), 1, stdin) == 1 &&
		    !N_BADMAG(exhdr)) {
			fseek(stdin,
			    (long )(N_TXTOFF(exhdr) + exhdr.a_text), 0);
			strings ((long )exhdr.a_data,
			    (long )(N_TXTOFF(exhdr) + exhdr.a_text));
		} else {
			fseek(stdin, 0L, 0);
			strings (-1L, 0L);
		}
	} while (argv[x] != NULL);
}

strings (count, loc)
	long count;
	long loc;
{
	char c, buf[BUFSIZ];
	char *bptr;

	bptr = buf;
	while (count--) {
		c = getchar();
		loc++;
		if (c == NULL || c == '\n' || bptr >= buf + BUFSIZ - 1) {
			if (bptr - buf > minlength) {
				*bptr = NULL;
				if (countflag)
					printf ("%7D\t", loc - 1 -
					    (long )( bptr - buf));
				printf ("%s\n", buf);
			}
			bptr = buf;
		} else
			if (c < 0177 && c >= ' ')
				*bptr++ = c;
			else
				bptr = buf;
		if (feof(stdin) || ferror(stdin))
			break;
	}
}
