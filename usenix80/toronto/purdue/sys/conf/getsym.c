/*
 * reverse of "strip", strips out code + data, writes
 * new a.out on std output with just symbols.
 * most code stolen from nm.
 */

int	fi;
int	fout;
int	buf[256];
main(argc, argv)
char **argv;
{
	int n, i, j;

	if (--argc > 0 && *argv[1] == '-') {
		argv++;
		while (*++*argv) switch (**argv) {

		default:
			continue;
		}
		argc--;
	}
	if (argc==0)
		fi = open("a.out", 0); else
		fi = open(*++argv, 0);
	if(fi < 0) {
		fout=2;
		printf("cannot open input\n");
		exit();
	}
	read(fi, buf, 020);
	if(buf[0]!=0407 && buf[0]!=0410 && buf[0]!=0411) {
		fout=2;
		printf("bad format\n");
		exit();
	}
	useek(fi, buf[1], 1);		/* text */
	useek(fi, buf[2], 1);		/* data */
	if(buf[7] != 1) {
		useek(fi, buf[1], 1);
		useek(fi, buf[2], 1);	/* reloc */
	}
	n = ldiv(0, buf[4], 12);
	if(n == 0) {
		fout=2;
		printf("no name list\n");
		exit();
	}
	buf[1] = buf[2] = 0; /* no text or data */
	write(1, buf, 020);	/* write new header */

	/* copy symbols to end of new file */

	while ((n=read(fi, buf, 512)) > 0)
		write(1, buf, n);
}
useek(f,offset,mode)
int f, offset, mode;
{       register int fd,t;
	fd = f;
	t = offset;
	if (t >= 0 || mode == 0 || mode == 3)
		return (seek (fd,offset,mode));
	else
	{       seek (fd,040000,mode);
		seek (fd,040000,mode);
		return (seek(fd,t&077777,mode));
	}
}
