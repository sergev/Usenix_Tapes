#define	SIZE	80

char    *
field(p, begin, end)
	char    *p;
	int     begin;
	int     end;
{
	register        int     i;
	register        int     j;
	static          char    buffer[SIZE];

	j = 0;
	for(i = begin ; i <= end ; i++)
		buffer[j++] = p[i];
	buffer[j] = '\0';

	if( j >= SIZE )
		printf("field: buffer overflow.  j = %d\n", j);

	return(buffer);
}
