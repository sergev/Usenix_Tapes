bcopy(s,d,l)
register char *s,*d;
register int l;
{
	while(--l>=0) *d++ = *s++;
}

bzero(d,l)
register char *d;
register int l;
{
	while(--l>=0) *d++ = 0;
}

bcmp(b1,b2,l)
register char *b1,*b2;
register int l;
{
	while(--l>=0) if(*b1++ != *b2++) return 1;
	return 0;
}
