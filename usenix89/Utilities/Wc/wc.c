/*
 *  wc [ -lwcp ] [ -spagesize ] [ -v ] [ files... ]
 *
 *  Count lines, words, characters, and pages.
 *
 *  Runs faster by doing less work if it doesn't have to count
 *  all quantities.
 *
 *  Use this program as you wish, but please leave this header intact.
 *
 *  Steve Summit 12/4/84
 */

#include <stdio.h>

#define TRUE 1
#define FALSE 0

long int totchars = 0;
long int totwords = 0;
long int totlines = 0;
long int totpages = 0;

#define LINES	04
#define WORDS	02
#define CHARS	01

int count = LINES | WORDS | CHARS;
char want[10] = "lwc";

int verbose = FALSE;

int pagelen = 66;

int errs = 0;

#define Isdigit(c) ((c) >= '0' && (c) <= '9')
#define Ctod(c) ((c) - '0')

#define Append(mask, letter)	if(deflt)				\
					{				\
					count = mask;			\
					(void)strcpy(want, letter);	\
					deflt = FALSE;			\
					}				\
				else	{				\
					count |= mask;			\
					(void)strcat(want, letter);	\
					}

#define Append2(letter)		if(deflt)				\
					{				\
					(void)strcpy(want, letter);	\
					deflt = FALSE;			\
					}				\
				else	(void)strcat(want, letter)

char *progname = "wc";

extern char *rindex();
extern char *strcat();
extern char *strcpy();

main(argc, argv)
int argc;
char *argv[];
{
int fd;
int deflt = TRUE;
int argi;
char *p;
int totals;

if(argc > 0)
	{
	p = rindex(argv[0], '/');
	if(p != NULL)
		progname = p + 1;
	else	progname = argv[0];
	}

for(argi = 1; argi < argc && argv[argi][0] == '-'; argi++)
	{
	for(p = &argv[argi][1]; *p != '\0'; p++)
		{
		switch(*p)
			{
			case 'l':
				Append(LINES, "l");
				break;

			case 'w':
				Append(WORDS, "w");
				break;

			case 'c':
				Append(CHARS, "c");
				break;

			case 'p':
				Append2("p");
				break;

			case 'v':
				verbose = TRUE;
				if(deflt)
					(void)strcpy(want, "lwcp");
				break;

			case 's':
				pagelen = 0;
				while(Isdigit(*(p + 1)))
					pagelen = 10 * pagelen + Ctod(*++p);
				break;

			default:
				fprintf(stderr, "%s: unknown option -%c\n",
								progname, *p);
			}
		}
	}

if(verbose)
	{
	for(p = want; *p != '\0'; p++)
		{
		switch(*p)
			{
			case 'l':
				printf("   lines");
				break;

			case 'w':
				printf("   words");
				break;

			case 'c':
				printf("   chars");
				break;

			case 'p':
				printf("   pages");
				break;
			}
		}

	putchar('\n');
	}

if(argi >= argc)
	wc("", 0);
else	{
	totals = (argi + 1) < argc;

	for(; argi < argc; argi++)
		{
		if((fd = open(argv[argi], 0)) < 0)
			{
			fprintf(stderr, "%s: can't open %s\n", progname,
								argv[argi]);
			perror("");
			errs++;
			continue;
			}
		wc(argv[argi], fd);
		(void)close(fd);
		}

	if(totals)
		{
		printit(totlines, totwords, totchars, totpages);
		printf(" total\n");
		}
	}

exit(errs);
}

#define Set(flag)	flag++
#define Clear(flag)	flag = FALSE

#define Checkline()	if(*p == '\n')					\
				lines++

#define Checkword()	if(' ' < *p && *p < '\177')			\
				{					\
				if(!inword)				\
					{				\
					words++;			\
					Set(inword);			\
					}				\
				continue;				\
				}

#define Checkword2()	else if(*p != ' ' && *p != '\t') 		\
				continue;				\
			Clear(inword)

#define Checkword3()	if(*p == ' ' || *p == '\n' || *p == '\t')	\
				Clear(inword)

#define Dochars()	chars += r

wc(name, fd)
char *name;
int fd;
{
char buf[BUFSIZ];
register char *bufend;
int r;
long int lines, words, chars, pages;
register char *p;
register int inword;

lines = words = chars = pages = 0;

Clear(inword);

switch(count)
	{
	case LINES:
		while((r = read(fd, buf, BUFSIZ)) > 0)
			{
			bufend = buf + r;
			for(p = buf; p < bufend; p++)
				Checkline();
			}
		break;

	case WORDS:
		while((r = read(fd, buf, BUFSIZ)) > 0)
			{
			bufend = buf + r;
			for(p = buf; p < bufend; p++)
				{
				Checkword();
				Checkword3();
				}
			}
		break;

	case CHARS:
		while((r = read(fd, buf, BUFSIZ)) > 0)
			Dochars();
		break;

	case LINES|CHARS:
		while((r = read(fd, buf, BUFSIZ)) > 0)
			{
			Dochars();

			bufend = buf + r;
			for(p = buf; p < bufend; p++)
				Checkline();
			}
		break;

	case LINES|WORDS:
		while((r = read(fd, buf, BUFSIZ)) > 0)
			{
			bufend = buf + r;
			for(p = buf; p < bufend; p++)
				{
				Checkword();
				Checkline();
				Checkword2();
				}
			}
		break;

	case WORDS|CHARS:
		while((r = read(fd, buf, BUFSIZ)) > 0)
			{
			Dochars();

			bufend = buf + r;
			for(p = buf; p < bufend; p++)
				{
				Checkword();
				Checkword3();
				}
			}
		break;

	case LINES|WORDS|CHARS:
		while((r = read(fd, buf, BUFSIZ)) > 0)
			{
			Dochars();

			bufend = buf + r;
			for(p = buf; p < bufend; p++)
				{
				Checkword();
				Checkline();
				Checkword2();
				}
			}
		break;
	}

if(r < 0)
	{
	fprintf(stderr, "%s: %s: read error\n", progname,
				*name != '\0' ? name : "standard input");
	perror("");
	errs++;
	}

pages = lines / pagelen + (lines % pagelen != 0 ? 1 : 0);

printit(lines, words, chars, pages);

if(*name != '\0')
	printf(" %s", name);
putchar('\n');

totlines += lines;
totwords += words;
totchars += chars;
totpages += pages;
}

printit(lines, words, chars, pages)
long int lines, words, chars, pages;
{
char *p;

for(p = want; *p != '\0'; p++)
	{
	switch(*p)
		{
		case 'l':
			printf(" %7ld", lines);
			break;

		case 'w':
			printf(" %7ld", words);
			break;

		case 'c':
			printf(" %7ld", chars);
			break;

		case 'p':
			printf(" %7ld", pages);
			break;
		}
	}
}
