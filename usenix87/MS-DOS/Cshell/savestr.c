#define NULL (void *)0
char *
savestr(s)
	register char *s;
{
	register char *r,*malloc();
	/* squirrel away matched file name */
	if (NULL == (r = malloc(strlen(s)+1)))
		abort();
	strcpy(r,s);
	r[strlen(s)] = '\0';
	return r;
}
