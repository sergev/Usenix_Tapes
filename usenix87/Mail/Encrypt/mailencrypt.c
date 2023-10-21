#include <stdio.h>

char	*Key, *Code;

main(argc, argv)
int	argc;
char	*argv[];
{
	char	line[1024];
	FILE	*fp;

	if (freopen(argv[1], "a", stdout) == NULL)
		exit(1);

	/*
	 * Copy the header untouched.
	 */
	while (fgets(line, sizeof(line), stdin) != NULL) {
		if (strlen(line) < 2)
			break;
		fputs(line, stdout);
	}

	fputs("Encrypted: vernam\n\n", stdout);
	fflush(stdout);
	SetKey();
	SetCode();
	Encrypt();
	Finish();
	putchar('\n');
	exit(0);
}

/*
 * This is pretty lame against someone
 * who is serious about breaking your mail
 * and has access to the binary of this program.
 * Set your Key and Code in these two routines.
 */
SetKey()
{
	Key = (char *)malloc(8);

	Key[0] = 't';
	Key[1] = 'o';
	Key[2] = 'u';
	Key[3] = 'l';
	Key[4] = 'o';
	Key[5] = 'u';
	Key[6] = 's';
	Key[7] = 'e';
	Key[8] = '\0';
}

SetCode()
{
	Code = (char *)malloc(17);

	Code[0] = '0';
	Code[1] = '1';
	Code[2] = '2';
	Code[3] = '3';
	Code[4] = '4';
	Code[5] = '5';
	Code[6] = '6';
	Code[7] = '7';
	Code[8] = '8';
	Code[9] = '9';
	Code[10] = 'A';
	Code[11] = 'B';
	Code[12] = 'C';
	Code[13] = 'D';
	Code[14] = 'E';
	Code[15] = 'F';
	Code[16] = '\0';
}


char	*Keyp;
int	Bcount, Ccount;

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
