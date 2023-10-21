/* Copy *s to *t, n bytes.
*/
copy(t,s,n)
 char *t, *s;
 int   n;
{
	while (n-- > 0)
		*t++ = *s++;
}
