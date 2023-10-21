/*
**      The old standby
*/

char    *
copy(from, to)
register char *from, *to;
{
	while (*to++ = *from++);
	return(--to);
}
