
#include <stdio.h>

#define	ROTORSIZ	256
#define	MASK		0377
#define	EMPTY		07777
#define	X_SIZE		4099

char	*strrchr();

unsigned	r1[ROTORSIZ];
unsigned	r2[ROTORSIZ];
unsigned	r3[ROTORSIZ];

unsigned char	x[X_SIZE];

init(password, decrypt)
char	*password;
int	decrypt;
{
	register int	index;
	register int	i;
	unsigned	random;
	long		seed = 123L;
	char		buf[14];

	strncpy(buf, password, 13);

	while (strlen(buf) < 13) strncat(buf, password, 13 - strlen(buf));

	for (i = 0 ; i < ROTORSIZ; i++) r1[i] = r2[i] = r3[i] = EMPTY;

	for (i = 0; i < 13; i++) seed = seed * buf[i] + i;

	i = 0;
	while (i < ROTORSIZ)
	{
		seed = (long)(5L * seed + (long)i);
		random = (unsigned)(seed % 65521L);
		index = (int)(random & MASK);
		if (r1[index] == EMPTY)
			r1[index] = i++;
		else
			continue;
	}

	i = 0;
	while (i < ROTORSIZ)
	{
		seed = (long)(5L * seed + (long)i);
		random = (unsigned)(seed % 65521L);
		index = (int)(random & MASK);
		if (r2[index] == EMPTY)
			r2[index] = i++;
		else
			continue;
	}

	i = 0;
	while (i < ROTORSIZ)
	{
		seed = (long)(5L * seed + (long)i);
		random = (unsigned)(seed % 65521L);
		index = (int)(random & MASK);
		if (r3[index] == EMPTY)
			r3[index] = i++;
		else
			continue;
	}

	for (i = 0; i < X_SIZE; i++)
	{
		seed = (long)(5L * seed + (long)i);
		random = (unsigned)(seed % 65521L);
		x[i] = random & 03;
	}

	if (decrypt)
	{
		invert(r1);
		invert(r2);
		invert(r3);
	}
}

invert(r)
unsigned r[ROTORSIZ];
{
	unsigned	t[ROTORSIZ];
	register int	i;

	for (i = 0; i < ROTORSIZ; i++) t[i] = r[i];
	for (i = 0; i < ROTORSIZ; i++) r[t[i]] = i;
}

crypt()
{
	register	int		ch;
	register	int		i    = 0;
	register	unsigned	ofs1 = 0;
	register	unsigned	ofs2 = 0;
	register	unsigned	ofs3 = 0;

	while ((ch = getchar()) != EOF)
	{
		putchar(r3[r2[r1[ch+ofs1&MASK]+ofs2&MASK]+ofs3&MASK]);

		switch (x[i]){
		case 00:
				ofs1 = ++ofs1 & MASK;
				break;
		case 01:
				ofs2 = ++ofs2 & MASK;
				break;
		case 02:
				ofs3 = ++ofs3 & MASK;
				break;
		}

		if (ofs1 == 0) ofs2 = ++ofs2 & MASK;
		if (ofs2 == 0) ofs3 = ++ofs3 & MASK;

		if (++i == X_SIZE) i = 0;
	}
}

decrypt()
{
	register	int		ch;
	register	int		i    = 0;
	register	unsigned	ofs1 = 0;
	register	unsigned	ofs2 = 0;
	register	unsigned	ofs3 = 0;

	while ((ch = getchar()) != EOF)
	{
		putchar(r1[r2[r3[ch]-ofs3&MASK]-ofs2&MASK]-ofs1&MASK);

		switch (x[i]){
		case 00:
				ofs1 = ++ofs1 & MASK;
				break;
		case 01:
				ofs2 = ++ofs2 & MASK;
				break;
		case 02:
				ofs3 = ++ofs3 & MASK;
				break;
		}

		if (ofs1 == 0) ofs2 = ++ofs2 & MASK;
		if (ofs2 == 0) ofs3 = ++ofs3 & MASK;

		if (++i == X_SIZE) i = 0;
	}
}

main(argc, argv)
int	argc;
char	*argv[];
{
	int	flag;
	char	*p;

	p = strrchr(argv[0], '/');

	if (p == NULL) p = argv[0];
	else ++p;

	if (strcmp(p, "crypt") == 0) flag = 0;
	else				   flag = 1;

	if (argc != 2)
		init(getpass("Enter key: "), flag);
	else
		init(argv[1], flag);

	if (flag) decrypt();
	else      crypt();
}
