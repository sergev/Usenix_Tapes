
       /*
	*       solve.c - A program to solve cryptarithmetic puzzles
	*
	*       (C) Copyright 1985 by Andreas Bormann (ab@unido.uucp)
	*
	*       Usage: solve [-v] <word1> <word2> <sum>
	*/

int     word1[50], word2[50], sum[51];

char    letter[11];
int     digit[11];
int     vflag, l, n1, n2, n3, cnt, sct;

main(argc, argv)
int argc;
char *argv[];
{
	register int    i,j,k;

	if(argc > 1 && strcmp("-v", argv[1]) == 0) {
		vflag++;
		argc--;
		argv++;
	}
	if(argc < 4) {
		printf("Usage: solve [-v] <word1> <word2> <sum>\n");
		exit(1);
	}
	i = j = k = l = 0;
	while(argv[1][i]) i++;
	while(argv[2][j]) j++;
	while(argv[3][k]) k++;
	do {
		word1[l] = i > 0 ? setletter(argv[1][--i]) : 10;
		word2[l] = j > 0 ? setletter(argv[2][--j]) : 10;
		sum[l]   = k > 0 ? setletter(argv[3][--k]) : 10;
		l++;
	} while(i > 0 || j > 0 || k > 0);
	sum[l] = word1[l] = word2[l] = 10;
	for(k = 0; k < 10; k++)
		digit[k] = -1;
	n1 = setletter(argv[1][0]);
	n2 = setletter(argv[2][0]);
	n3 = setletter(argv[3][0]);
	nextdigit(0);
	printf("Search finished after %d tests and %d partial solutions.\n",
		cnt, sct);
	exit(0);
}

setletter(c)
register char c;
{
	register int    k;

	for(k = 0; k < 10; k++) {
		if(letter[k] == c)
			return(k);
		if(letter[k] == 0) {
			letter[k] = c;
			return(k);
		}
	}
	printf("No more than 10 different letters allowed!\n");
	exit(1);
}

nextdigit(d)
{
	register        i,k;

	if(d == 10)
		return;
	sct++;
	if(vflag) {
		print(word1, l, ' ');
		print(word2, l, '+');
		for(i = 0; i <= l; i++)
			putchar('=');
		putchar('\n');
		print(sum, l, ' ');
		printf("\n");
	}
	for(k = 0; k < 10; k++) {
		if(k == 0 && (d == n1 || d == n2 || d == n3))
			goto nextk;
		for(i = 0; i < d; i++)
			if(digit[i] == k) goto nextk;
		digit[d] = k;
		if(addok())
			nextdigit(d+1);
	 nextk:
		;
	}
	digit[d] = -1;
}

addok()
{
	register int    k, w1, w2, s, s1, carry, i;

	cnt++;
	k = 0;
	carry = 0;
	while(word1[k] != 10 || word2[k] != 10 || sum[k] != 10) {
		if((w1 = digit[word1[k]]) == -1)
			return(1);
		if((w2 = digit[word2[k]]) == -1)
			return(1);
		if((s = digit[sum[k]]) == -1)
			return(1);
		s1 = w1 + w2 + carry;
		if(s1 > 9) {
			s1 -= 10;
			carry = 1;
		} else
			carry = 0;
		if(s != s1)
			return(0);
		k++;
	}
	if(carry)
		return(0);
	printf("Solution found after %d tests and %d partial solutions:\n\n",
		cnt, sct);
	printf("%c = %d", letter[0], digit[0]);
	for(i = 1; letter[i]; i++) 
		printf(", %c = %d", letter[i], digit[i]);
	printf("\n\n");
	print(word1, k, ' ');
	print(word2, k, '+');
	for(i = 0; i <= k; i++)
		putchar('=');
	putchar('\n');
	print(sum, k, ' ');
	printf("\n\n");
	return(0);
}

print(z, k, s)
int z[], k;
char s;
{
	char    c;

	putchar(s);
	while(k-- > 0) {
		if((c = digit[z[k]]) == -1)
			c = letter[z[k]];
		else
			c += '0';
		if(z[k] == 10)
			c = ' ';
		putchar(c);
	}
	putchar('\n');
}
