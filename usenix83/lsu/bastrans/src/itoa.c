itoa(n,s)
char s[];
int n;
{
	int i,sign;
	if ((sign=n) < 0) /* record sign */
		n = -n;
	i=0;
	do {		/* generate digits in reverse order */
		s[i++] = n % 10 + '0';	/* get next digit */
	} while ((n /= 10) > 0);	/* delete */
	if (sign < 0)
		s[i++] = '-';
	s[i] = '\0';
	reverse(s);
}

reverse(s)
char s[];
{
	int c,i,j;
	for (i=0,j=strlen(s)-1;i<j;i++,j--) {
		c=s[i];
		s[i]=s[j];
		s[j]=c;
	}
}
