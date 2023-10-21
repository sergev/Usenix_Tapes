#include <stdio.h>
char	*Key;
char	*Keyp;
char	*Code;
int	Bcount, Ccount;

main(argc, argv)
int	argc;
char	*argv[];
{
	FILE	*fpin, *fpout;
	register len;
	register decrypt = 0;

	argc--, argv++;
	while (argc > 0 && **argv == '-') {
		register char *cp = *argv++;
		argc--;
		while (*++cp) switch (*cp) {
			case 'k': 
				len = strlen(*argv);
				Key = (char *)strcpy(malloc(len + 1), *argv);
				bzero(*argv, len);
				argc--, argv++;
				break;
			case 'c':
				len = strlen(*argv);
				Code = (char *)strcpy(malloc(len + 1), *argv);
				bzero(*argv, len);
				argc--, argv++;
				break;
			case 'e':
				decrypt = 0;
				break;
			case 'd':
				decrypt++;
				break;
			default:
				fprintf(stderr, "Unknown option: %c\n", *cp);
				exit(1);
		}
	}


	if (Key == (char *)0)
		SetupKey();
	if (Code == (char *)0)
		SetupCode();

	if (*argv) {
		if ((fpin = freopen(*argv, "r", stdin)) == NULL) {
			perror(*argv);
			exit(1);
		}
		argv++;
	}

	if (*argv) {
		if ((fpout = freopen(*argv, "w", stdout)) == NULL) {
			perror(*argv);
			exit(1);
		}
		argv++;
	}

	if (decrypt) {
		Decrypt();
	} else {
		Encrypt();
		Finish();
	}

	exit(0);
}

SetupKey()
{
	static char	line[1024];
	char	*ptr;

	fprintf(stderr, "Key: ");
	fflush(stderr);
	Getline(line, sizeof(line));

	Key = line;
}

SetupCode()
{
	register	len;
	static char	line[20];

	do {
		fprintf(stderr, "Code: ");
		fflush(stderr);
		Getline(line, sizeof(line));
		len = strlen(line);
	} while (len < 16);
	line[16] = 0;
	Code = line;
}

Getline(line, length)
char	*line;
{
	char	*ptr;
	FILE	*fp;

	if ((fp = fopen("/dev/tty", "r")) == NULL) {
		perror("/dev/tty");
		exit(1);
	}
	if (fgets(line, length, fp) == NULL) {
		fclose(fp);
		return (1);
	}
	if ((ptr = (char *)index(line, '\n')) != NULL)
		*ptr = 0;
	fclose(fp);
	return (0);
}

Encrypt()
{
	register char	c;

	while ((c = getchar()) != EOF && !(ferror(stdin) || feof(stdin)))
		EnCode(Cypher(c));
}

/* Vernam Cypher */
Cypher(c)
char	c;
{
	if (Keyp == 0 || *Keyp == '\0')
		Keyp = Key;

	return (c ^ *Keyp++);
}

EnCode(c)
char	c;
{
	Output(Code[(unsigned int)(c & 0xf0) >> 4]);
	Output(Code[(unsigned int)(c & 0x0f)]);
}

Output(c, fp)
char	c;
FILE	*fp;
{
	putchar(c);

	if (++Ccount >= 6) {
		Ccount = 0;
		if (++Bcount >= 10) {
			Bcount = 0;
			putchar('\n');
		} else
			putchar(' ');
	}
}

Finish()
{
	long	t;

	time(&t);
	srand(t);

	while (Bcount)
		EnCode(Cypher(Code[rand() & 0xf]));
}

Decrypt()
{
	register char	c;

	while ((c = DeCode()) != -1)
		putchar(Cypher(c));

	putchar('\n');
}

DeCode()
{
	unsigned char	hi, lo, c;
	char	*phi, *plo;

	if ((hi = Input()) == -1)
		return (-1);
	if ((phi = (char *)index(Code, hi)) == NULL)
		return (-1);

	if ((lo = Input()) == -1 || (plo = (char *)index(Code, lo)) == NULL)
		return (-1);

	hi = phi - Code;
	lo = plo - Code;

	return (hi << 4 | lo);
}

Input()
{
	register char	c;

	if ((c = getchar()) == EOF && (ferror(stdin) || feof(stdin)))
		return(-1);

	if (c == ' ' || c == '\n')
		return (Input());

	return (c);
}
