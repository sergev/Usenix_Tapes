int spaces 2;
int indent 0;
char nofile 1;        /* If true, no file has been specified yet */
struct buf
{       int fildes;
	int nleft;
	char *nextp;
	char buffer[512];
} bf;

main (argc, argv)
int argc;
char *argv[];
{       extern fout;
	register char *p;

	fout = dup(1);
	while (argc-- > 1)
	{       argv++;
		if (**argv == '-')
		{       p = *argv;
			if (*++p == 'i')
				indent = integ(++p);
			else
				spaces = integ(p);
		}
		else
		{       if (fopen(*argv, &bf) == -1)
				printf ("Can't open %s\n", argv[1]);
			else
				putfile();
			nofile = 0;
		}
	}
	if (nofile) putfile();  /* Standard input if no other files */
}

putfile()
{       register int icolpos, ocolpos, c;
	int i, j;

	icolpos = ocolpos = 0;
	while ((c = getc(&bf)) != -1)
	{       switch (c)
		{ case '\n':
			for (i = spaces; i-->0 ; )
				putchar(c);
			icolpos = ocolpos = 0;
			break;
		  case '\r':
		  case '\014': /* Form Feed */
			putchar(c);
			icolpos = ocolpos = 0;
			break;
		  case ' ':
			icolpos++;
			break;
		  case '\t':
			icolpos = (icolpos+8)&~07;
			break;
		  default:
			if ((i = icolpos + indent - ocolpos) > 0)
			{       while ((j = (((ocolpos+8)&~07)-ocolpos)) <= i)
				{       putchar('\t');
					ocolpos =+ j;
					i =- j;
				}
				while (i--)
				{       putchar(' ');
					ocolpos++;
				}
			}
			if (c == '\036' || c == '\010') /* Backspaces */
			{       if (icolpos)
				{       icolpos--;
					ocolpos--;
					putchar(c);
				}
			} else if ((0 < c && c < ' ')   /* Other Controls */
			     || (0200 < c && c < 0240))
				putchar(c);
			else
			{       icolpos++;
				ocolpos++;
				putchar(c);
			}
		}
	}
	flush();
	close (bf.fildes);
}

integ(pp)
char *pp;
{       register char *p;
	register int i;
	i = 0;
	for (p = pp; *p>='0' && *p<='9'; p++)
		i = 10*i + *p - '0';
	return (i);
}
