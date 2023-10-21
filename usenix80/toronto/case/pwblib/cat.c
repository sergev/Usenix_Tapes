char *_cat_[] "~|^`cat.c	1.2";

cat(dest,s1)
char *dest;
register char **s1;
{
	register char *d,*s;
	register char **source;

	d = dest;
	source = &s1;
	while(s = *source++){
		while (*d++ = *s++) ;
		d--;
	}
	return(dest);
}
